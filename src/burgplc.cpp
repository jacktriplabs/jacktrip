#include "burgplc.h"
#include <sstream>
#include "jacktrip_globals.h"

using namespace std;
#define TWOTOTHETENTH 1024
#define TWOTOTHESIXTEENTH 65536
#define OUT(x,ch,s) sampleToBits(x,ch,s)
#define PACKETSAMP ( int s = 0; s < mFPP; s++ )

BurgPLC::BurgPLC(int sample_rate, int channels, int bit_res, int FPP, int qLen, int hist) :
    RingBuffer(0, 0),
    mSampleRate (sample_rate),
    mNumChannels (channels),
    mAudioBitRes (bit_res),
    mFPP (FPP),
    mQLen (qLen),
    mHist (hist)
{
    switch (mAudioBitRes) { // int from JitterBuffer to AudioInterface enum
    case 1: mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT8;
        break;
    case 2: mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT16;
        break;
    case 3: mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT24;
        break;
    case 4: mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT32;
        break;
    }
    mTotalSize = mSampleRate * mNumChannels * mAudioBitRes * 2;  // 2 secs of audio
    mXfrBuffer   = new int8_t[mTotalSize];
    mPacketCnt = 0; // burg
#define TRAINSAMPS (mHist*mFPP)
#define ORDER (TRAINSAMPS-1)
    mTrain.resize( TRAINSAMPS, 0.0 );
    mPrediction.resize( TRAINSAMPS-1, 0.0 ); // ORDER
    mCoeffs.resize( TRAINSAMPS-2, 0.0 );
    mTruth.resize( mFPP, 0.0 );
    mTruthCh1.resize( mFPP, 0.0 );
    mXfadedPred.resize( mFPP, 0.0 );
    mNextPred.resize( mFPP, 0.0 );
    mLastGoodPacket.resize( mFPP, 0.0 );
    for ( int i = 0; i < mHist; i++ ) {
        vector<sample_t> tmp( mFPP, 0.0 );
        mLastPackets.push_back(tmp);
    }
    mFadeUp.resize( mFPP, 0.0 );
    mFadeDown.resize( mFPP, 0.0 );
    for ( int i = 0; i < mFPP; i++ ) {
        mFadeUp[i] = (double)i/(double)mFPP;
        mFadeDown[i] = 1.0 - mFadeUp[i];
    }
    mLastWasGlitch = false;
    mPhasor.resize( mNumChannels, 0.0 );
    mIncomingSeq = -1;
    mOutgoingCnt = -1;
    mLastIncomingSeq = 0;
    mOutgoingCntWraps = 0;
    //    mIncomingSeqWrap.resize(TWOTOTHESIXTEENTH);
    //    for ( int i = 0; i < TWOTOTHESIXTEENTH; i++ ) mIncomingSeqWrap[i] = -1;
    mBytes = mFPP*mNumChannels*mBitResolutionMode;
    for ( int i = 0; i < TWOTOTHETENTH; i++ ) {
        int8_t* tmp = new int8_t[mBytes];
        mIncomingDat.push_back(tmp);
    }
    mLastOutgoingSeq = 0;
    mUnderrunCtr = 0;
    mZeros = new int8_t[mTotalSize];
    for PACKETSAMP OUT(0.0, 0, s);
    for PACKETSAMP OUT(0.0, 1, s);
    memcpy(mZeros, mXfrBuffer, mBytes);
    mUnderSig = new int8_t[mTotalSize];
    for PACKETSAMP OUT(((s==0)||(s==mFPP-1))?-0.5:-0.3, 0, s);
    for PACKETSAMP OUT(-0.3, 1, s);
    memcpy(mUnderSig, mXfrBuffer, mBytes);
    mOverSig = new int8_t[mTotalSize];
    for PACKETSAMP OUT(((s==0)||(s==mFPP-1))?0.5:0.3, 0, s);
    for PACKETSAMP OUT(0.3, 1, s);
    memcpy(mOverSig, mXfrBuffer, mBytes);
    mBalance = 0;
    mTimer0.start();
    mTimer1.start();
    mTimer2.start();
    mTimer3.start();
    mElapsedAcc = 0.0;
    mExpectedOutgoingSeq = 0;
    mLastOutgoingCnt = 0;
    mLastIncomingSeq2 = 0;
    mIncomingCnt = 0;
    mIncomingCntWraps = 0;
    mIncomingCntWrap.resize(TWOTOTHETENTH);
    for ( int i = 0; i < TWOTOTHETENTH; i++ ) mIncomingCntWrap[i] = -1;
    mLastPush = 0.0;
    mStopped = false;
    mCur = 0;
    mPushed = false;
    mDelta = 0;
    mLastIncomingCnt = 0;
    mStat = new Stat;
    mStat->window = 1000; // fast ticks
    start();
}
void BurgPLC::stats(Stat *stat, double msNow)
{ // stdDev based on mean of last windowful
    QMutexLocker locker(&mMutex);
    if (stat->ctr%stat->window) {
        if (mDelta<stat->min) stat->min = mDelta;
        else if (mDelta>stat->max) stat->max = mDelta;
        stat->acc += mDelta;
        double tmp = mDelta - stat->mean; // last window
        stat->var += (tmp*tmp);
        stat->ctr++;
    } else {
        stat->mean = (double)stat->acc / (double) stat->window;
        stat->var /= stat->window;
        stat->stdDev = sqrt(stat->var);

        if (stat->acc) {
            QString out;
            out += (QString::number(msNow) + QString("\t"));
            out += (QString::number(stat->mean) + QString("\t"));
            out += (QString::number(stat->min) + QString("\t"));
            out += (QString::number(stat->max) + QString("\t"));
            emit printStats(out);
        }

        stat->acc = 0;
        stat->var = 0;
        stat->min = 999999999;
        stat->max = -stat->max;
        stat->ctr = 0;
    }
}
void BurgPLC::run()
{
    setRealtimeProcessPriority2();
    while (!mStopped) {
        plot();
        usleep(15); // = 17 usec
    }
}

void BurgPLC::plot()
{
    QMutexLocker locker(&mMutex); // lock the mutex here and pullPacket but not pushPacket
    mDelta = mIncomingCnt - mOutgoingCnt;
    if ((!(mIncomingCnt%TWOTOTHESIXTEENTH))&&(mOutgoingCnt!=-1)) mIncomingCnt = mOutgoingCnt;
    //    mCur++;
    QString out;
    double elapsed0 = (double)mTimer0.nsecsElapsed() / 1000000.0;
    if (mOutgoingCnt==-1) return;
    //    else if (elapsed0<1000.0) return;
    double elapsed3 = (double)mTimer3.nsecsElapsed() / 1000000.0;
    out += (QString::number(elapsed0) + QString("\t"));
    out += (QString::number(elapsed3) + QString("\t"));
    out += (QString::number(0) + QString("\t"));
    out += (QString::number(mDelta) + QString("\t"));
    mTimer3.start();
//    emit print(out);
    stats(mStat, elapsed0);
    if (mLastIncomingCnt != mIncomingCnt) {
        //        if(false) {
        mLastIncomingCnt = mIncomingCnt;
        double push = (double)mTimer1.nsecsElapsed() / 1000000.0;
        //        double ipi = push - mLastPush;
        //        if(mIncomingCnt %= TWOTOTHETENTH)
        //            mElapsedAcc += abs(ipi); else {
        ////            qDebug() << elapsed0 << (mElapsedAcc / (double)TWOTOTHETENTH);
        //            mElapsedAcc = 0.0;
        //        }
        mLastPush = push;
        mCur = mIncomingCnt % TWOTOTHETENTH;
        mIncomingCntWrap[mCur] = mIncomingCntWraps;
        memcpy(mIncomingDat[mCur], mUDPbuf, mBytes);
        memcpy(mXfrBuffer, mIncomingDat[mCur], mBytes);
        inputPacket();
        mTimer1.start();
        QString out;
        out += (QString::number(elapsed0) + QString("\t"));
        out += (QString::number(push) + QString("\t"));
        out += (QString::number(1) + QString("\t")); // blk
        out += (QString::number(mDelta) + QString("\t"));
        //        emit print(out);
        mPushed = true;
        return;
    }
    //    if (mLastOutgoingCnt != mOutgoingCnt) {
    if(false) {
        mLastOutgoingCnt = mOutgoingCnt;
        double pull = (double)mTimer2.nsecsElapsed() / 1000000.0;

        int lag = mCur-mQLen;
        if (lag<0) lag+=TWOTOTHETENTH;
        int test = mIncomingCntWraps;
        if (lag<0) test--;
        //        processPacket(mIncomingCntWrap[lag]!=test);
        //        processPacket(mIncomingCntWrap[mCur]!=mIncomingCntWraps);
        if (!mPushed) {
            processPacket(true);
        }
        //        else processPacket(false);
        memcpy(mJACKbuf, mXfrBuffer, mBytes);
        mTimer2.start();
        QString out;
        out += (QString::number(elapsed0) + QString("\t"));
        out += (QString::number(pull) + QString("\t"));
        out += (QString::number(2) + QString("\t")); // grn
        out += (QString::number(mDelta) + QString("\t"));
        emit print(out);
        mPushed = false;
        return;
    }
}

bool BurgPLC::pushPacket (const int8_t *buf, int len, int seq) {
    QMutexLocker locker(&mMutex);
    mIncomingSeq = seq % TWOTOTHETENTH;
    mIncomingCntWraps = seq / TWOTOTHETENTH;
    int nextSeq = mLastIncomingSeq2+1;
    nextSeq %= TWOTOTHETENTH;
    if (mIncomingSeq != nextSeq) qDebug() << "LOST PACKET" << mIncomingSeq << nextSeq;
    mLastIncomingSeq2 = mIncomingSeq;
    if (!mIncomingCnt) qDebug() << "twice";
    if (!mIncomingCnt) mUDPbuf=buf;
    if (mIncomingSeq!=-1) mIncomingCnt++;
    return true;
};

void BurgPLC::pullPacket (int8_t* buf) {
    QMutexLocker locker(&mMutex);
    mOutgoingCntWraps = mOutgoingCnt / TWOTOTHETENTH;
    //    if (!mOutgoingCntWraps)
    mJACKbuf=buf;
    if (mIncomingSeq!=-1)  {
        if (mOutgoingCnt==-1) {
            qDebug() << "once";
            mOutgoingCnt = mIncomingCnt;
        }
        else mOutgoingCnt++; // will saturate
    }
};

#define RUN 3

void BurgPLC::inputPacket ()
{
#define INCh0(x,s) mTruth[s] = x
#define INCh1(x,s) mTruthCh1[s] = x
    for PACKETSAMP {
        INCh0(bitsToSample(0, s), s);
        INCh1(bitsToSample(1, s), s);
    }
    //                for PACKETSAMP {
    //                    INCh0(0.3*sin(mPhasor[0]), s);
    //                    INCh1(0.3*sin(mPhasor[1]), s);
    //                    mPhasor[0] += 0.1;
    //                    mPhasor[1] += 0.11;
    //                }
    if(mPacketCnt) {
        if(RUN > 2) {

            for ( int i = 0; i < mHist; i++ ) {
                for PACKETSAMP mTrain[s+((mHist-(i+1))*mFPP)] =
                        mLastPackets[i][s];
            }
        }
    }
}

void BurgPLC::processPacket (bool glitch)
{
    //    QMutexLocker locker(&mMutex);  // lock the mutex
    // debugSequenceDelta = bytesToInt(mXfrBuffer) - debugSequenceNumber;

    //    if (debugSequenceNumber != bytesToInt(mXfrBuffer))
    //        qDebug() << "expected" << debugSequenceNumber << "got " << bytesToInt(mXfrBuffer);

    //if(!overrun)
    //    debugSequenceNumber = bytesToInt(mXfrBuffer)+1;
    //    if (debugSequenceDelta != 1) qDebug() << debugSequenceNumber << "\t" << debugSequenceDelta;


    //    glitch = !(mPacketCnt%100);
    if(mPacketCnt) {
        if(RUN > 2) {
            //            if (glitch) qDebug() << "glitch";

            // GET LINEAR PREDICTION COEFFICIENTS
            ba.train( mCoeffs, mTrain, mPacketCnt );

            // LINEAR PREDICT DATA
            vector<sample_t> tail( mTrain );

            ba.predict( mCoeffs, tail ); // resizes to TRAINSAMPS-2 + TRAINSAMPS

            for ( int i = 0; i < ORDER; i++ )
                mPrediction[i] = tail[i+TRAINSAMPS];
            ///////////////////////////////////////////// // CALCULATE AND DISPLAY ERROR

            for PACKETSAMP mXfadedPred[s] = mTruth[s] * mFadeUp[s] + mNextPred[s] * mFadeDown[s];
        }
        for PACKETSAMP {
            switch(RUN)
            {
            case -1  :
                OUT(0.3*sin(mPhasor[0]), 0, s);
                OUT(0.3*sin(mPhasor[1]), 1, s);
                mPhasor[0] += 0.1;
                mPhasor[1] += 0.11;
                break;
            case 0  :
                OUT(mTruth[s], 0, s);
                OUT(mTruthCh1[s], 1, s);
                break;
            case 1  : OUT((glitch) ?
                              mLastGoodPacket[s] : mTruth[s], 0, s);
                OUT(mTruthCh1[s], 1, s);
                break;
            case 2  : {
                double tmp = (glitch) ? 0.0 : mTruth[s];
                OUT(tmp, 0, s);
                OUT(mTruthCh1[s], 1, s);
            }
                break;
            case 3  :
                OUT((glitch) ? mPrediction[s] : ( (mLastWasGlitch) ?
                                                      mXfadedPred[ s ] : mTruth[s] ), 0, s);
                //                OUT(mTruthCh1[s], 1, s);
                OUT(0.0, 1, s);
                break;
            case 4  :
                OUT((glitch) ? mPrediction[s] : mTruth[s], 0, s);
                break;
            case 5  : OUT(mPrediction[s], 0, s);
                break;
            case 6  :
                OUT(0.3*sin(mPhasor[0]), 0, s);
                OUT(0.3*sin(mPhasor[1]), 1, s);
                mPhasor[0] += 0.1;
                mPhasor[1] += 0.11;
                break;
            }
        }
        mLastWasGlitch = glitch;
        for PACKETSAMP
                mNextPred[s] = mPrediction[ s + mFPP];
    }

    // if mPacketCnt==0 initialization follows
    for ( int i = mHist-1; i>0; i-- ) {
        for PACKETSAMP mLastPackets[i][s] = mLastPackets[i-1][s];
    }

    // will only be able to glitch if mPacketCnt>0
    for PACKETSAMP mLastPackets[0][s] =
            ((!glitch)||(mPacketCnt<mHist)) ? mTruth[s] : mPrediction[s];

    if (!glitch)
        for PACKETSAMP mLastGoodPacket[s] = mTruth[s];
    mPacketCnt++;
}


// copped from AudioInterface.cpp

sample_t BurgPLC::bitsToSample(int ch, int frame) {
    sample_t sample = 0.0;
    AudioInterface::fromBitToSampleConversion(
                &mXfrBuffer[(frame * mBitResolutionMode * mNumChannels)
            + (ch * mBitResolutionMode)],
            &sample, mBitResolutionMode);
    return sample;
}

void BurgPLC::sampleToBits(sample_t sample, int ch, int frame) {
    AudioInterface::fromSampleToBitConversion(
                &sample,
                &mXfrBuffer[(frame * mBitResolutionMode * mNumChannels)
            + (ch * mBitResolutionMode)],
            mBitResolutionMode);
}

QString BurgPLC::qStringFromLongDouble(const long double myLongDouble)
{
    stringstream ss;
    ss << myLongDouble;
    return QString::fromStdString(ss.str());
}

// returns the first stereo frame of a buffer as an int32
int BurgPLC::bytesToInt(const int8_t *buf)
{
    int output = 0;
    memcpy(&output, buf, sizeof(int));  // 4 bytes
    return output;
}
