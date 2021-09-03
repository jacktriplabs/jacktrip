//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2021 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file PoolBuffer.cpp
 * \author Chris Chafe
 * \date May 2021
 */

// EXPERIMENTAL for testing in JackTrip v1.4.0
// runs ok from FPP 16 up to 512, but don't try 1024 yet
// in / out channels are the same -- mono, stereo and -n3 tested fine

// for example, server
// jacktrip -S --udprt  -p1 --bufstrategy 3 -q2
// and client
// jacktrip -C cmn9.stanford.edu --udprt --bufstrategy 3 -q2

#include "PoolBuffer.h"
#include <iomanip>
#include <QTextStream>
#include <sstream>

#include "jacktrip_globals.h"
using std::cout;
using std::endl;
using std::setw;

#define MINFPP 16
#define STDDEVINDOW     200  // packets
#define STDDEV2POOLSIZE 20.0
#define MAXPOOLSIZE     6000  // insanely large pool, 2 sec FPP16
#define HIST            6     // at FPP32
#define OUT(x, ch, s)   sampleToBits(x, ch, s)
// from listening tests iteration performs better than '=' operator
#define PACKETSAMP (int s = 0; s < mFPP; s++)

//*******************************************************************************
PoolBuffer::PoolBuffer(int sample_rate, int channels, int bit_res, int FPP, int qLen)
    : RingBuffer(0, 0)
    , mNumChannels(channels)
    , mAudioBitRes(bit_res)
    , mFPP(FPP)
    , mSampleRate(sample_rate)
    , mPoolSize(qLen)
    , mQlen(qLen)
{
    switch (mAudioBitRes) {  // int from JitterBuffer to AudioInterface enum
    case 1:
        mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT8;
        break;
    case 2:
        mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT16;
        break;
    case 3:
        mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT24;
        break;
    case 4:
        mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT32;
        break;
    }
    mMinPoolSize = mQlen + 2;
    mPoolSize = mMinPoolSize;
    mHist            = 6 * 32;                // samples, from original settings
    double histFloat = mHist / (double)mFPP;  // packets for other FPP
    mHist            = (int)histFloat;
    if (mHist < 2)
        mHist = 2;  // min packets, needs a pair
    else if (mHist > 6)
        mHist = 6;  // max packets, keep a lid on CPU load
    if (gVerboseFlag) cout << "mHist = " << mHist << " at " << mFPP << "\n";
    //    qDebug() << "mHist =" << mHist << "@" << mFPP;
    mBytes     = mFPP * mNumChannels * mBitResolutionMode;
    mXfrBuffer = new int8_t[mBytes];
    mPacketCnt = 0;  // burg
    mFadeUp.resize(mFPP, 0.0);
    mFadeDown.resize(mFPP, 0.0);
    for (int i = 0; i < mFPP; i++) {
        mFadeUp[i]   = (double)i / (double)mFPP;
        mFadeDown[i] = 1.0 - mFadeUp[i];
    }
    mLastWasGlitch = false;
    mOutgoingCnt   = 0;

    mMaxPoolSize = mPoolSize;
    for (int i = 0; i < mMaxPoolSize; i++) {
        int8_t* tmp = new int8_t[mBytes];
        mIncomingDat.push_back(tmp);
    }
    mIndexPool.resize(mMaxPoolSize);
    for (int i = 0; i < mMaxPoolSize; i++) mIndexPool[i] = -1;

    mIncomingCnt = 0;
    mTimer0 = new QElapsedTimer();
    mTimer0->start();
    mGlitchCnt = 0;
    for (int i = 0; i < mNumChannels; i++) {
        ChanData* tmp = new ChanData(i, mFPP, mHist);
        mChanData.push_back(tmp);
        for PACKETSAMP OUT(0.0, i, s); // zero mXfrBuffer
    }
    mZeros = new int8_t[mBytes];
    memcpy(mZeros, mXfrBuffer, mBytes);
    stdDev = new StdDev( (int)(floor(48000.0 / (double)mFPP)), 1);
    stdDev2 = new StdDev( (int)(floor(48000.0 / (double)mFPP)), 2);
    mFPPfactor = STDDEV2POOLSIZE * (0.0 + (32 / (double)mFPP));
    mLastLostCount = 0;
    tmpCtr = 0;
    tmpTimer = new QElapsedTimer();
    mLastSeqNum = -1;
    mLastSeqNumOut = 1;
    mModSeqNum = 65536;
}

//*******************************************************************************
bool PoolBuffer::pushPacket(const int8_t* buf, int seq_num)
{
    QMutexLocker locker(&mMutex);
    //    qDebug() << "pushPacket" << seq_num;
    if((mLastSeqNum != -1) && (((mLastSeqNum+1)%mModSeqNum) != seq_num)) qDebug() << "lost packet detected in pushPacket" << seq_num << mLastSeqNum;
    mLastSeqNum = seq_num;

    int freeSlot = -1;
    for (int i = 0; i < mPoolSize; i++) {
        if (mIndexPool[i] == -1) {
            freeSlot = i;
            break;
        }
    }
    if (freeSlot == -1) {
        int oldest      = 2*mModSeqNum;
        int oldestIndex = -2;
        for (int i = 0; i < mPoolSize; i++) {
            if (mIndexPool[i]+mModSeqNum < oldest) {
                oldest = mIndexPool[i]+mModSeqNum;
                oldestIndex = i;
            }
        }
        freeSlot = oldestIndex;
        //        qDebug() << "pool overflow -- erasing"  << mIndexPool[freeSlot] << "at"  << freeSlot;
    }
    if (freeSlot == -2) qDebug() << "pool overflow -- trouble erasing"  << "freeSlot"  << freeSlot;
    mIndexPool[freeSlot] = seq_num;
    memcpy(mIncomingDat[freeSlot], buf, mBytes);
    return true;
};

//*******************************************************************************
//    (/ (* (- (expt 2 32) 1) (/ 32 48000.0)) (* 60 60 24))
void PoolBuffer::pullPacket(int8_t* buf)
{
    QMutexLocker locker(&mMutex);
    int slot = -1;
    int lag = mQlen;
    while (lag && (slot == -1)) {
        for (int i = 0; i < mPoolSize; i++) {
            int tmp = mLastSeqNum-lag;
            if (tmp<0) tmp+=mModSeqNum;
            if (mIndexPool[i] == tmp) {
                slot = i;
                break;
            }
        }
        lag--;
    }

    if (mLastSeqNum != -1) {
        if (slot == -1) {
            qDebug() << "missing mLastSeqNum" << mLastSeqNum;
            processPacket(true);
        }
        else {
            //        qDebug() << "lag" << lag;
            //        fprintf(stderr,"%d\t", lag);             fflush(stderr);
            memcpy(mXfrBuffer, mIncomingDat[slot], mBytes);
            processPacket(false);
            mIndexPool[slot] = -1;
        }
    } else {
        memcpy(mXfrBuffer, mZeros, mBytes);
    }
    memcpy(buf, mXfrBuffer, mBytes);
};

//*******************************************************************************
void PoolBuffer::processPacket(bool glitch)
{
    for (int ch = 0; ch < mNumChannels; ch++)
        processChannel(ch, glitch, mPacketCnt, mLastWasGlitch);
    mLastWasGlitch = glitch;
    mPacketCnt++;
}

//*******************************************************************************
void PoolBuffer::processChannel(int ch, bool glitch, int packetCnt, bool lastWasGlitch)
{
    //    if(glitch) qDebug() << "glitch"; else fprintf(stderr,".");

    ChanData* cd = mChanData[ch];
    for
            PACKETSAMP cd->mTruth[s] = bitsToSample(ch, s);
            if (packetCnt) {
                for (int i = 0; i < mHist; i++) {
                    for
                            PACKETSAMP cd->mTrain[s + ((mHist - (i + 1)) * mFPP)] =
                                    cd->mLastPackets[i][s];
                }
                if (glitch) {
                    // GET LINEAR PREDICTION COEFFICIENTS
                    ba.train(cd->mCoeffs, cd->mTrain);

                    // LINEAR PREDICT DATA
                    //                vector<sample_t> tail(cd->mTrain);
                    mTail = cd->mTrain;

                    //                ba.predict(cd->mCoeffs, tail);  // resizes to TRAINSAMPS-2 + TRAINSAMPS
                    ba.predict(cd->mCoeffs, mTail);  // resizes to TRAINSAMPS-2 + TRAINSAMPS

                    for (int i = 0; i < (cd->trainSamps - 1); i++)
                        //                    cd->mPrediction[i] = tail[i + cd->trainSamps];
                        cd->mPrediction[i] = mTail[i + cd->trainSamps];
                }
                if (lastWasGlitch) for PACKETSAMP
                        cd->mXfadedPred[s] = cd->mTruth[s] * mFadeUp[s] + cd->mNextPred[s] * mFadeDown[s];

                for PACKETSAMP
                        OUT((glitch) ? cd->mPrediction[s]
                                       : ((lastWasGlitch) ? cd->mXfadedPred[s] : cd->mTruth[s]),
                            ch, s);

                if (glitch) {
                    for PACKETSAMP cd->mNextPred[s] = cd->mPrediction[s + mFPP];
                }
            }

            // if mPacketCnt==0 initialization follows
            for (int i = mHist - 1; i > 0; i--) {
                for PACKETSAMP
                        cd->mLastPackets[i][s] = cd->mLastPackets[i - 1][s];
            }

            // will only be able to glitch if mPacketCnt>0
            for PACKETSAMP cd->mLastPackets[0][s] =
                    ((!glitch) || (packetCnt < mHist)) ? cd->mTruth[s] : cd->mPrediction[s];
}

//*******************************************************************************
// copped from AudioInterface.cpp

sample_t PoolBuffer::bitsToSample(int ch, int frame)
{
    sample_t sample = 0.0;
    AudioInterface::fromBitToSampleConversion(
                &mXfrBuffer[(frame * mBitResolutionMode * mNumChannels)
            + (ch * mBitResolutionMode)],
            &sample, mBitResolutionMode);
    return sample;
}

void PoolBuffer::sampleToBits(sample_t sample, int ch, int frame)
{
    AudioInterface::fromSampleToBitConversion(
                &sample,
                &mXfrBuffer[(frame * mBitResolutionMode * mNumChannels)
            + (ch * mBitResolutionMode)],
            mBitResolutionMode);
}

//*******************************************************************************
bool BurgAlgorithm::classify(double d)
{
    bool tmp = false;
    switch (fpclassify(d)) {
    case FP_INFINITE:
        qDebug() << ("infinite");
        tmp = true;
        break;
    case FP_NAN:
        qDebug() << ("NaN");
        tmp = true;
        break;
    case FP_ZERO:
        //      qDebug() <<  ("zero");
        tmp = true;
        break;
    case FP_SUBNORMAL:
        qDebug() << ("subnormal");
        tmp = true;
        break;
        //    case FP_NORMAL:    qDebug() <<  ("normal");    break;
    }
    //  if (signbit(d)) qDebug() <<  (" negative\n"); else qDebug() <<  (" positive or
    //  unsigned\n");
    return tmp;
}

void BurgAlgorithm::train(vector<long double>& coeffs, const vector<float>& x)
{
    // GET SIZE FROM INPUT VECTORS
    size_t N = x.size() - 1;
    size_t m = coeffs.size();

    //        if (x.size() < m) qDebug() << "time_series should have more elements than
    //        the AR order is";

    // INITIALIZE Ak
    //    vector<long double> Ak(m + 1, 0.0);
    Ak.assign(m + 1, 0.0);
    Ak[0] = 1.0;

    // INITIALIZE f and b
    //    vector<long double> f;
    f.resize(x.size());
    for (unsigned int i = 0; i < x.size(); i++) f[i] = x[i];
    //    vector<long double> b(f);
    b = f;

    // INITIALIZE Dk
    long double Dk = 0.0;
    for (size_t j = 0; j <= N; j++)  // CC: N is $#x-1 in C++ but $#x in perl
    {
        Dk += 2.00001 * f[j] * f[j];  // CC: needs more damping than orig 2.0
    }
    Dk -= f[0] * f[0] + b[N] * b[N];

    //    qDebug() << "Dk" << qStringFromLongDouble1(Dk);
    //        if ( classify(Dk) )
    //        { qDebug() << pCnt << "init";
    //        }

    // BURG RECURSION
    for (size_t k = 0; k < m; k++) {
        // COMPUTE MU
        long double mu = 0.0;
        for (size_t n = 0; n <= N - k - 1; n++) { mu += f[n + k + 1] * b[n]; }

        if (Dk == 0.0) Dk = 0.0000001;  // CC: from testing, needs eps
        //            if ( classify(Dk) ) qDebug() << pCnt << "run";

        mu *= -2.0 / Dk;
        //            if ( isnan(Dk) )  { qDebug() << "k" << k; }
        //            if (Dk==0.0) qDebug() << "k" << k << "Dk==0";

        // UPDATE Ak
        for (size_t n = 0; n <= (k + 1) / 2; n++) {
            long double t1 = Ak[n] + mu * Ak[k + 1 - n];
            long double t2 = Ak[k + 1 - n] + mu * Ak[n];
            Ak[n]          = t1;
            Ak[k + 1 - n]  = t2;
        }

        // UPDATE f and b
        for (size_t n = 0; n <= N - k - 1; n++) {
            long double t1 = f[n + k + 1] + mu * b[n];  // were double
            long double t2 = b[n] + mu * f[n + k + 1];
            f[n + k + 1]   = t1;
            b[n]           = t2;
        }

        // UPDATE Dk
        Dk = (1.0 - mu * mu) * Dk - f[k + 1] * f[k + 1] - b[N - k - 1] * b[N - k - 1];
    }
    // ASSIGN COEFFICIENTS
    coeffs.assign(++Ak.begin(), Ak.end());
}

void BurgAlgorithm::predict(vector<long double>& coeffs, vector<float>& tail)
{
    size_t m = coeffs.size();
    //    qDebug() << "tail.at(0)" << tail[0]*32768;
    //    qDebug() << "tail.at(1)" << tail[1]*32768;
    tail.resize(m + tail.size());
    //    qDebug() << "tail.at(m)" << tail[m]*32768;
    //    qDebug() << "tail.at(...end...)" << tail[tail.size()-1]*32768;
    //    qDebug() << "m" << m << "tail.size()" << tail.size();
    for (size_t i = m; i < tail.size(); i++) {
        tail[i] = 0.0;
        for (size_t j = 0; j < m; j++) { tail[i] -= coeffs[j] * tail[i - 1 - j]; }
    }
}

//*******************************************************************************
ChanData::ChanData(int i, int FPP, int hist) : ch(i)
{
    trainSamps = (hist * FPP);
    mTruth.resize(FPP, 0.0);
    mXfadedPred.resize(FPP, 0.0);
    mNextPred.resize(FPP, 0.0);
    for (int i = 0; i < hist; i++) {
        vector<sample_t> tmp(FPP, 0.0);
        mLastPackets.push_back(tmp);
    }
    mTrain.resize(trainSamps, 0.0);
    mPrediction.resize(trainSamps - 1, 0.0);  // ORDER
    mCoeffs.resize(trainSamps - 2, 0.0);
}

//*******************************************************************************
StdDev::StdDev(int w, int id) : window(w), mId(id)
{
    reset();
    longTermStdDev    = 0.0;
    longTermStdDevAcc = 0.0;
    longTermCnt       = 0;
    lastMean          = 0.0;
    lastMin           = 0;
    lastMax           = 0;
    mTimer            = new QElapsedTimer();
    mTimer->start();
    data.resize(w, 0.0);
}

void StdDev::reset()
{
    mean = 0.0;
    //        varRunning = 0.0;
    acc = 0.0;
    min = 999999.0;
    max = 0.0;
    ctr = 0;
    glitches = 0;
    balance = 0;
};

double StdDev::tick()
{
    double msElapsed = (double)mTimer->nsecsElapsed() / 1000000.0;
    mTimer->start();
    if (ctr != window) {
        data[ctr] = msElapsed;
        if (msElapsed < min)
            min = msElapsed;
        else if (msElapsed > max)
            max = msElapsed;
        acc += msElapsed;
        ctr++;
    } else {
        mean       = (double)acc / (double)window;
        double var = 0.0;
        for (int i = 0; i < window; i++) {
            double tmp = data[i] - mean;
            var += (tmp * tmp);
        }
        var /= (double)window;
        double stdDev = sqrt(var);
        if (longTermCnt) {
            longTermStdDevAcc += stdDev;
            longTermStdDev = longTermStdDevAcc / (double)longTermCnt;
            //            qDebug() << mean << min << max << stdDev << longTermStdDev;
            if (gVerboseFlag)
                cout << setw(10) << mean << setw(10) << min << setw(10) << max << setw(10)
                     << stdDev << setw(10) << longTermStdDev << endl;
        } else if (gVerboseFlag)
            cout << "printing directly from PoolBuffer->stdDev->tick:\n (mean / min / max / "
                    "stdDev / longTermStdDev) \n";

        longTermCnt++;
        //            QString out;
        //            out += (QString::number(msNow) + QString("\t"));
        //            out += (QString::number(mean) + QString("\t"));
        //            out += (QString::number(min) + QString("\t"));
        //            out += (QString::number(max) + QString("\t"));
        //            out += (QString::number(stdDev) + QString("\t"));
        //                        emit printStats(out);
        // build-jacktrip-Desktop-Release/jacktrip -C cmn9.stanford.edu --bufstrategy 3 -I
        // 1 -G /tmp/iostat.log plot 'iostat.log' u  1:2 w l, 'iostat.log' u  1:3 w l,
        // 'iostat.log' u  1:4 w l, 'iostat.log' u  1:5 w l,
        lastMean = mean;
        lastMin  = min;
        lastMax  = max;
        lastStdDev = stdDev;
        reset();
    }
    return msElapsed;
}

//*******************************************************************************
QString PoolBuffer::getStats(uint32_t statCount, uint32_t lostCount)
{
    // formatting floats in columns looks better with std::stringstream than with QTextStream
    QString tmp;
    if (!statCount) {
        tmp = QString("PoolBuffer: inter-packet intervals msec\n");
        tmp += "      (window of last ";
        tmp += QString::number(stdDev2->window);
        tmp += " packets)\n";
        tmp += "secs   (mean       min       max     stdDev)   avgStdDev  balance  plc   poolsize   q   lost\n";
    }
    else {
        uint32_t lost = lostCount - mLastLostCount;
        mLastLostCount = lostCount;
#define PDBL(x) << setw(10) << (QString("%1").arg(stdDev->x, 0, 'f', 2)).toStdString()
#define PDBL2(x) << setw(10) << (QString("%1").arg(stdDev2->x, 0, 'f', 2)).toStdString()
        std::stringstream logger;
        logger << setw(2) << statCount
                  PDBL(lastMean)
                  PDBL(lastMin)
                  PDBL(lastMax)
                  PDBL(lastStdDev)
                  PDBL(longTermStdDev)
               << endl;
        tmp = QString::fromStdString(logger.str());
        std::stringstream logger2;
        logger2 << setw(2) << ""
                   PDBL2(lastMean)
                   PDBL2(lastMin)
                   PDBL2(lastMax)
                   PDBL2(lastStdDev)
                   PDBL2(longTermStdDev)
                << setw(8) << stdDev2->balance
                << setw(8) << stdDev2->glitches
                << setw(8) << mPoolSize
                << setw(8) << mQlen
                << setw(8) << lost << endl;
        tmp += QString::fromStdString(logger2.str());
    }
    return tmp;
}

