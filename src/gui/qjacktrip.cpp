//*****************************************************************
/*
  QJackTrip: Bringing a graphical user interface to JackTrip, a
  system for high quality audio network performance over the
  internet.

  Copyright (c) 2020 Aaron Wyatt.

  This file is part of QJackTrip.

  QJackTrip is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  QJackTrip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with QJackTrip.  If not, see <https://www.gnu.org/licenses/>.
*/
//*****************************************************************

#include "qjacktrip.h"

#include <QFileDialog>
#include <QHostAddress>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QVector>
#include <cstdlib>
#include <ctime>

#include "about.h"
#include "ui_qjacktrip.h"
#ifdef USE_WEAK_JACK
#include "weak_libjack.h"
#endif

#ifdef RT_AUDIO
#include "../RtAudioInterface.h"
#include "RtAudio.h"
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
#define QCHECKBOX_STATE_CHANGED QCheckBox::stateChanged
#else
#define QCHECKBOX_STATE_CHANGED QCheckBox::checkStateChanged
#endif

#include "../Compressor.h"
#include "../CompressorPresets.h"
#include "../Limiter.h"
#include "../Meter.h"
#include "../Reverb.h"

QJackTrip::QJackTrip(UserInterface& interface, QWidget* parent)
    : QMainWindow(parent)
    , m_interface(interface)
    , m_ui(new Ui::QJackTrip)
    , m_netManager(new QNetworkAccessManager(this))
    , m_statsDialog(new MessageDialog(this, QStringLiteral("Stats")))
    , m_debugDialog(new MessageDialog(this, QStringLiteral("Debug"), 2))
    , m_realCout(std::cout.rdbuf())
    , m_realCerr(std::cerr.rdbuf())
    , m_jackTripRunning(false)
    , m_isExiting(false)
    , m_exitSent(false)
    , m_hideWarning(false)
{
    m_ui->setupUi(this);

    // Set up our debug window, and relay everything to our real cout.
    std::cout.rdbuf(m_debugDialog->getOutputStream()->rdbuf());
    std::cerr.rdbuf(m_debugDialog->getOutputStream(1)->rdbuf());
    m_debugDialog->setRelayStream(&m_realCout);
    m_debugDialog->setRelayStream(&m_realCerr, 1);

    // Create all our UI connections.
    connect(m_ui->typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &QJackTrip::chooseRunType);
    connect(m_ui->addressComboBox, &QComboBox::currentTextChanged, this,
            &QJackTrip::addressChanged);
    connect(m_ui->connectButton, &QPushButton::clicked, this, &QJackTrip::start);
    connect(m_ui->disconnectButton, &QPushButton::clicked, this, &QJackTrip::stop);
    connect(m_ui->exitButton, &QPushButton::clicked, this, &QJackTrip::exit);
    connect(m_ui->certBrowse, &QPushButton::clicked, this, &QJackTrip::browseForFile);
    connect(m_ui->keyBrowse, &QPushButton::clicked, this, &QJackTrip::browseForFile);
    connect(m_ui->credsBrowse, &QPushButton::clicked, this, &QJackTrip::browseForFile);
    connect(m_ui->commandLineButton, &QPushButton::clicked, this,
            &QJackTrip::showCommandLineMessageBox);
    connect(m_ui->useDefaultsButton, &QPushButton::clicked, this,
            &QJackTrip::resetOptions);
    connect(m_ui->usernameEdit, &QLineEdit::textChanged, this,
            &QJackTrip::credentialsChanged);
    connect(m_ui->passwordEdit, &QLineEdit::textChanged, this,
            &QJackTrip::credentialsChanged);
    connect(m_ui->certEdit, &QLineEdit::textChanged, this, &QJackTrip::authFilesChanged);
    connect(m_ui->keyEdit, &QLineEdit::textChanged, this, &QJackTrip::authFilesChanged);
    connect(m_ui->credsEdit, &QLineEdit::textChanged, this, &QJackTrip::authFilesChanged);
    connect(m_ui->aboutButton, &QPushButton::clicked, this, [this]() {
        About about(this);
        about.exec();
    });
#ifdef NO_VS
    m_ui->authNotVSLabel->setText(
        QStringLiteral("(This is for JackTrip's inbuilt authentication system. To easily "
                       "connect to a Virtual Studio server, download a Virtual Studio "
                       "enabled version of JackTrip.)"));
    m_ui->vsModeButton->setVisible(false);
#else
    connect(m_ui->vsModeButton, &QPushButton::clicked, this,
            &QJackTrip::virtualStudioMode);
    m_ui->vsModeButton->setVisible(true);
#endif
    connect(m_ui->autoPatchComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() {
                if (m_ui->autoPatchComboBox->currentIndex() == CLIENTFOFI
                    || m_ui->autoPatchComboBox->currentIndex() == FULLMIX) {
                    m_ui->patchServerCheckBox->setEnabled(true);
                } else {
                    m_ui->patchServerCheckBox->setEnabled(false);
                }
            });
    connect(m_ui->authCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        m_ui->usernameLabel->setEnabled(m_ui->authCheckBox->isChecked());
        m_ui->usernameEdit->setEnabled(m_ui->authCheckBox->isChecked());
        m_ui->passwordLabel->setEnabled(m_ui->authCheckBox->isChecked());
        m_ui->passwordEdit->setEnabled(m_ui->authCheckBox->isChecked());
        credentialsChanged();
    });
    connect(m_ui->requireAuthCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        m_ui->certLabel->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->certEdit->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->certBrowse->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->keyLabel->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->keyEdit->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->keyBrowse->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->credsLabel->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->credsEdit->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->credsBrowse->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        authFilesChanged();
    });
    connect(m_ui->ioStatsCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        m_ui->ioStatsLabel->setEnabled(m_ui->ioStatsCheckBox->isChecked());
        m_ui->ioStatsSpinBox->setEnabled(m_ui->ioStatsCheckBox->isChecked());
        if (!m_ui->ioStatsCheckBox->isChecked()) {
            m_statsDialog->hide();
        }
    });
    connect(m_ui->verboseCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        gVerboseFlag = m_ui->verboseCheckBox->isChecked();
        if (!gVerboseFlag) {
            m_debugDialog->hide();
            m_debugDialog->clearOutput();
        }
    });
    connect(m_ui->jitterCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        m_ui->broadcastCheckBox->setEnabled(m_ui->jitterCheckBox->isChecked());
        m_ui->broadcastQueueLabel->setEnabled(m_ui->jitterCheckBox->isChecked()
                                              && m_ui->broadcastCheckBox->isChecked());
        m_ui->broadcastQueueSpinBox->setEnabled(m_ui->jitterCheckBox->isChecked()
                                                && m_ui->broadcastCheckBox->isChecked());
        m_ui->bufferStrategyLabel->setEnabled(m_ui->jitterCheckBox->isChecked());
        m_ui->bufferStrategyComboBox->setEnabled(m_ui->jitterCheckBox->isChecked());
        // m_ui->strategyExplanationLabel->setEnabled(m_ui->jitterCheckBox->isChecked());
        m_ui->bufferLine->setEnabled(m_ui->jitterCheckBox->isChecked());
        m_ui->autoQueueCheckBox->setEnabled(m_ui->jitterCheckBox->isChecked());
        m_ui->autoQueueLabel->setEnabled(m_ui->jitterCheckBox->isChecked()
                                         && m_ui->autoQueueCheckBox->isChecked());
        m_ui->autoQueueSpinBox->setEnabled(m_ui->jitterCheckBox->isChecked()
                                           && m_ui->autoQueueCheckBox->isChecked());
        m_ui->packetsLabel->setEnabled(m_ui->jitterCheckBox->isChecked()
                                       && m_ui->autoQueueCheckBox->isChecked());
        m_ui->autoQueueExplanationLabel->setEnabled(
            m_ui->jitterCheckBox->isChecked() && m_ui->autoQueueCheckBox->isChecked());
        if (m_ui->jitterCheckBox->isChecked() && m_ui->autoQueueCheckBox->isChecked()) {
            m_autoQueueIndicator.setText(QStringLiteral("Auto queue: enabled"));
        } else {
            m_autoQueueIndicator.setText(QStringLiteral("Auto queue: disabled"));
        }
    });
    connect(m_ui->broadcastCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        m_ui->broadcastQueueLabel->setEnabled(m_ui->jitterCheckBox->isChecked()
                                              && m_ui->broadcastCheckBox->isChecked());
        m_ui->broadcastQueueSpinBox->setEnabled(m_ui->jitterCheckBox->isChecked()
                                                && m_ui->broadcastCheckBox->isChecked());
    });
    connect(m_ui->autoQueueCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        m_ui->autoQueueLabel->setEnabled(m_ui->jitterCheckBox->isChecked()
                                         && m_ui->autoQueueCheckBox->isChecked());
        m_ui->autoQueueSpinBox->setEnabled(m_ui->jitterCheckBox->isChecked()
                                           && m_ui->autoQueueCheckBox->isChecked());
        m_ui->packetsLabel->setEnabled(m_ui->jitterCheckBox->isChecked()
                                       && m_ui->autoQueueCheckBox->isChecked());
        m_ui->autoQueueExplanationLabel->setEnabled(
            m_ui->jitterCheckBox->isChecked() && m_ui->autoQueueCheckBox->isChecked());
        if (m_ui->jitterCheckBox->isChecked() && m_ui->autoQueueCheckBox->isChecked()) {
            m_autoQueueIndicator.setText(QStringLiteral("Auto queue: enabled"));
        } else {
            m_autoQueueIndicator.setText(QStringLiteral("Auto queue: disabled"));
        }
    });

    connect(m_ui->inFreeverbCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        m_ui->inFreeverbLabel->setEnabled(m_ui->inFreeverbCheckBox->isChecked());
        m_ui->inFreeverbWetnessSlider->setEnabled(m_ui->inFreeverbCheckBox->isChecked());
    });
    connect(m_ui->inZitarevCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        m_ui->inZitarevLabel->setEnabled(m_ui->inZitarevCheckBox->isChecked());
        m_ui->inZitarevWetnessSlider->setEnabled(m_ui->inZitarevCheckBox->isChecked());
    });

    connect(m_ui->outFreeverbCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        m_ui->outFreeverbLabel->setEnabled(m_ui->outFreeverbCheckBox->isChecked());
        m_ui->outFreeverbWetnessSlider->setEnabled(
            m_ui->outFreeverbCheckBox->isChecked());
    });
    connect(m_ui->outZitarevCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        m_ui->outZitarevLabel->setEnabled(m_ui->outZitarevCheckBox->isChecked());
        m_ui->outZitarevWetnessSlider->setEnabled(m_ui->outZitarevCheckBox->isChecked());
    });
    connect(m_ui->outLimiterCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        m_ui->outLimiterLabel->setEnabled(m_ui->outLimiterCheckBox->isChecked());
        m_ui->outClientsSpinBox->setEnabled(m_ui->outLimiterCheckBox->isChecked());
    });

    connect(m_ui->connectScriptCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        m_ui->connectScriptEdit->setEnabled(m_ui->connectScriptCheckBox->isChecked());
        m_ui->connectScriptBrowse->setEnabled(m_ui->connectScriptCheckBox->isChecked());
    });
    connect(m_ui->disconnectScriptCheckBox, &QCHECKBOX_STATE_CHANGED, this, [this]() {
        m_ui->disconnectScriptEdit->setEnabled(
            m_ui->disconnectScriptCheckBox->isChecked());
        m_ui->disconnectScriptBrowse->setEnabled(
            m_ui->disconnectScriptCheckBox->isChecked());
    });
    connect(m_ui->connectScriptBrowse, &QPushButton::clicked, this,
            &QJackTrip::browseForFile);
    connect(m_ui->disconnectScriptBrowse, &QPushButton::clicked, this,
            &QJackTrip::browseForFile);

    m_ui->statusBar->showMessage(QStringLiteral("JackTrip version ").append(gVersion));

    // Set up our interface for the default Client run mode.
    //(loadSettings will take care of the UI in all other cases.)
    m_ui->basePortLabel->setVisible(false);
    m_ui->basePortSpinBox->setVisible(false);
    m_ui->autoPatchGroupBox->setVisible(false);
    m_ui->requireAuthGroupBox->setVisible(false);
    m_ui->backendWarningLabel->setVisible(false);
    m_ui->inputGroupBox->setVisible(false);
    m_ui->outputGroupBox->setVisible(false);

    m_inputLayout.reset(new QGridLayout(m_ui->inputGroupBox));
    m_outputLayout.reset(new QGridLayout(m_ui->outputGroupBox));

#ifdef RT_AUDIO
    connect(m_ui->backendComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                if (index == 1) {
                    m_ui->sampleRateComboBox->setEnabled(true);
                    m_ui->sampleRateLabel->setEnabled(true);
                    m_ui->bufferSizeComboBox->setEnabled(true);
                    m_ui->bufferSizeLabel->setEnabled(true);
                    m_ui->inputDeviceComboBox->setEnabled(true);
                    m_ui->inputDeviceLabel->setEnabled(true);
                    m_ui->outputDeviceComboBox->setEnabled(true);
                    m_ui->outputDeviceLabel->setEnabled(true);
                    m_ui->refreshDevicesButton->setEnabled(true);
                    m_ui->backendWarningLabel->setVisible(true);
                    populateDeviceMenu(m_ui->inputDeviceComboBox, true);
                    populateDeviceMenu(m_ui->outputDeviceComboBox, false);
                } else {
                    m_ui->sampleRateComboBox->setEnabled(false);
                    m_ui->sampleRateLabel->setEnabled(false);
                    m_ui->bufferSizeComboBox->setEnabled(false);
                    m_ui->bufferSizeLabel->setEnabled(false);
                    m_ui->inputDeviceComboBox->setEnabled(false);
                    m_ui->inputDeviceLabel->setEnabled(false);
                    m_ui->outputDeviceComboBox->setEnabled(false);
                    m_ui->outputDeviceLabel->setEnabled(false);
                    m_ui->refreshDevicesButton->setEnabled(false);
                    m_ui->backendWarningLabel->setVisible(false);
                }
            });
    connect(m_ui->refreshDevicesButton, &QPushButton::clicked, this, [this]() {
        populateDeviceMenu(m_ui->inputDeviceComboBox, true);
        populateDeviceMenu(m_ui->outputDeviceComboBox, false);
    });
#else
    int tabIndex = findTab(QStringLiteral("Audio Backend"));
    if (tabIndex != -1) {
        m_ui->optionsTabWidget->removeTab(tabIndex);
    }
#endif

    migrateSettings();
    m_ui->optionsTabWidget->setCurrentIndex(0);

    QVector<QLabel*> labels;
    labels << m_ui->inFreeverbLabel << m_ui->inZitarevLabel << m_ui->outFreeverbLabel;
    std::srand(std::time(nullptr));
    int index = std::rand() % 4;
    if (index < 3) {
        labels.at(index)->setToolTip(m_ui->outZitarevLabel->toolTip());
        m_ui->outZitarevLabel->setToolTip(QLatin1String(""));
    }
}

void QJackTrip::closeEvent(QCloseEvent* event)
{
    if (!m_exitSent) {
        // Ignore the close event so that we can override the handling of it.
        event->ignore();
        exit();
    }
}

void QJackTrip::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    // We need to fix the layout of our word wrapped labels.
    // The font should be the same for all of them so we can reuse the one QFontMetrics
    // object
    QFontMetrics metrics(m_ui->autoQueueExplanationLabel->font());
    // This seems like a convoluted way to get what is effectively our layout geometry,
    // but until we look at the jitter tab, the layout geometry is unset.
    int width = m_ui->JitterTab->contentsRect().width()
                - m_ui->JitterTab->contentsMargins().left()
                - m_ui->JitterTab->contentsMargins().right()
                - m_ui->JitterTab->layout()->contentsMargins().left()
                - m_ui->JitterTab->layout()->contentsMargins().right();

    /*QRect rect = metrics.boundingRect(0, 0, width, 0, Qt::TextWordWrap,
    m_ui->strategyExplanationLabel->text());
    m_ui->strategyExplanationLabel->setMinimumHeight(rect.height());*/
    QRect rect = metrics.boundingRect(0, 0, width, 0, Qt::TextWordWrap,
                                      m_ui->autoQueueExplanationLabel->text());
    m_ui->autoQueueExplanationLabel->setMinimumHeight(rect.height());

    width = m_ui->requireAuthGroupBox->contentsRect().width()
            - m_ui->requireAuthGroupBox->contentsMargins().left()
            - m_ui->requireAuthGroupBox->contentsMargins().right()
            - m_ui->requireAuthGroupBox->layout()->contentsMargins().left()
            - m_ui->requireAuthGroupBox->contentsMargins().right();
    rect = metrics.boundingRect(0, 0, width, 0, Qt::TextWordWrap,
                                m_ui->authDisclaimerLabel->text());
    m_ui->authDisclaimerLabel->setMinimumHeight(rect.height());

    width = m_ui->authGroupBox->contentsRect().width()
            - m_ui->authGroupBox->contentsMargins().left()
            - m_ui->authGroupBox->contentsMargins().right()
            - m_ui->authGroupBox->layout()->contentsMargins().left()
            - m_ui->authGroupBox->contentsMargins().right();
    rect = metrics.boundingRect(0, 0, width, 0, Qt::TextWordWrap,
                                m_ui->authNotVSLabel->text());
    m_ui->authNotVSLabel->setMinimumHeight(rect.height());

    // The previous minimum heights should protect any further word wrapped labels,
    // but it's worth including any additional ones here for future proofing.
    width = m_ui->scriptingTab->contentsRect().width()
            - m_ui->scriptingTab->contentsMargins().left()
            - m_ui->scriptingTab->contentsMargins().right()
            - m_ui->scriptingTab->layout()->contentsMargins().left()
            - m_ui->scriptingTab->contentsMargins().right();
    rect = metrics.boundingRect(0, 0, width, 0, Qt::TextWordWrap,
                                m_ui->environmentVariableLabel->text());
    m_ui->environmentVariableLabel->setMinimumHeight(rect.height());
}

void QJackTrip::showEvent(QShowEvent* event)
{
    // We need to wait to load geometry until here rather than with our other settings.
    // If we don't, the window geometry will be improperly set on macOS whenever the
    // VirtualStudio window is shown first.
    QMainWindow::showEvent(event);
    if (m_firstShow) {
        QSettings settings;
        loadSettings(&m_interface.getSettings());

        // Display a warning about any ignored command line options.
        if (m_interface.getSettings().guiIgnoresArguments()) {
            QMessageBox msgBox;
            msgBox.setText(
                "You have supplied command line options that the GUI version of JackTrip "
                "currently ignores. (Everything else will run as expected.)\n\nRun "
                "\"jacktrip -h\" for more details.");
            msgBox.setWindowTitle(QStringLiteral("Command line options"));
            msgBox.exec();
        }

        // Add an autoqueue indicator to the status bar.
        m_ui->statusBar->addPermanentWidget(&m_autoQueueIndicator);
        if (m_ui->jitterCheckBox->isChecked() && m_ui->autoQueueCheckBox->isChecked()) {
            m_autoQueueIndicator.setText(QStringLiteral("Auto queue: enabled"));
        } else {
            m_autoQueueIndicator.setText(QStringLiteral("Auto queue: disabled"));
        }

#ifdef USE_WEAK_JACK
        // Check if Jack is actually available
        if (have_libjack() != 0) {
#ifdef RT_AUDIO
            bool usingRtAudioAlready = m_ui->backendComboBox->currentIndex() == 1;
            m_ui->backendComboBox->setCurrentIndex(1);
            m_ui->backendComboBox->setEnabled(false);
            m_ui->backendLabel->setEnabled(false);

            // If we're in Hub Server mode, switch us back to P2P server mode.
            if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
                m_ui->typeComboBox->setCurrentIndex(P2P_SERVER);
            }
            m_ui->typeComboBox->removeItem(HUB_SERVER);
            m_ui->backendWarningLabel->setText(
                "JACK was not found. This means that only the RtAudio backend is "
                "available and that JackTrip cannot be run in hub server mode.");

            settings.beginGroup(QStringLiteral("Audio"));
            if (!settings.value(QStringLiteral("HideJackWarning"), false).toBool()) {
                QCheckBox* dontBugMe =
                    new QCheckBox(QStringLiteral("Don't show this warning again"));
                QMessageBox msgBox;
                msgBox.setText(
                    "An installation of JACK was not found. JackTrip will still run "
                    "using a different audio backend (RtAudio) but some more advanced "
                    "features, like the ability to run your own hub server, will not be "
                    "available.\n\n(If you install JACK at a later stage, these features "
                    "will automatically be re-enabled.)");
                msgBox.setWindowTitle(QStringLiteral("JACK Not Available"));
                msgBox.setCheckBox(dontBugMe);
                QObject::connect(dontBugMe, &QCHECKBOX_STATE_CHANGED, this,
                                 [this, dontBugMe]() {
                                     m_hideWarning = dontBugMe->isChecked();
                                 });
                msgBox.exec();
                if (m_hideWarning) {
                    settings.setValue(QStringLiteral("HideJackWarning"), true);
                }
                if (!usingRtAudioAlready) {
                    settings.setValue(QStringLiteral("UsingFallback"), true);
                }
            }
            settings.endGroup();
        } else {
            // If we've fallen back to RtAudio before and JACK is now installed, use JACK.
            settings.beginGroup(QStringLiteral("Audio"));
            if (settings.value(QStringLiteral("UsingFallback"), false).toBool()) {
                m_ui->backendComboBox->setCurrentIndex(0);
                settings.setValue(QStringLiteral("UsingFallback"), false);
            }
            settings.endGroup();
#else   // RT_AUDIO
            QMessageBox msgBox;
            msgBox.setText(
                "An installation of JACK was not found, and no other audio backends are "
                "available. JackTrip will not be able to start. (Please install JACK to "
                "fix this.)");
            msgBox.setWindowTitle("JACK Not Available");
            msgBox.exec();
#endif  // RT_AUDIO
        }
#endif  // USE_WEAK_JACK

        settings.beginGroup(QStringLiteral("Window"));
        QByteArray geometry = settings.value(QStringLiteral("Geometry")).toByteArray();
        if (geometry.size() > 0) {
            restoreGeometry(geometry);
        } else {
            // Because of hidden elements in our dialog window, it's vertical size in the
            // creator is getting rediculous. Set it to something sensible by default if
            // this is our first load.
            this->resize(QSize(this->size().height(), 600));
        }
        settings.endGroup();

        // Use the ipify API to find our external IP address.
        connect(m_netManager.data(), &QNetworkAccessManager::finished, this,
                &QJackTrip::receivedIP);
        m_netManager->get(QNetworkRequest(QUrl(QStringLiteral("https://api.ipify.org"))));
        m_netManager->get(
            QNetworkRequest(QUrl(QStringLiteral("https://api6.ipify.org"))));
        m_firstShow = false;
    }
}

void QJackTrip::processFinished()
{
    if (!m_jackTripRunning) {
        // Don't execute this if our process isn't actually running.
        return;
    }
    m_jackTripRunning = false;
    m_interface.enableNap();
    m_ui->disconnectButton->setEnabled(false);
    if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
        m_udpHub.reset();
    } else {
        m_jackTrip.reset();
    }

    if (m_ui->disconnectScriptCheckBox->isChecked()) {
        QStringList arguments = m_ui->disconnectScriptEdit->text().split(
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringLiteral(" "), Qt::SkipEmptyParts);
#else
            QStringLiteral(" "), QString::SkipEmptyParts);
#endif
        if (!arguments.isEmpty()) {
            QProcess disconnectScript;
            disconnectScript.setProgram(arguments.takeFirst());
            disconnectScript.setWorkingDirectory(QDir::homePath());
            disconnectScript.setArguments(arguments);
            disconnectScript.setStandardOutputFile(QProcess::nullDevice());
            disconnectScript.setStandardErrorFile(QProcess::nullDevice());
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert(QStringLiteral("JT_CLIENT_NAME"), m_assignedClientName);
            env.insert(QStringLiteral("JT_SEND_CHANNELS"),
                       QString::number(m_ui->channelSendSpinBox->value()));
            env.insert(QStringLiteral("JT_RECV_CHANNELS"),
                       QString::number(m_ui->channelRecvSpinBox->value()));
            disconnectScript.setProcessEnvironment(env);
            disconnectScript.startDetached();
        }
    }

    if (m_isExiting) {
        m_exitSent = true;
        emit signalExit();
    } else {
        enableUi(true);
        m_ui->connectButton->setEnabled(true);
        m_ui->statusBar->showMessage(QStringLiteral("JackTrip Processes Stopped"), 2000);
    }
}

void QJackTrip::processError(const QString& errorMessage)
{
    QMessageBox msgBox;
    if (errorMessage == QLatin1String("Peer Stopped")) {
        // Report the other end quitting as a regular occurrence rather than an error.
        msgBox.setText(errorMessage);
        msgBox.setWindowTitle(QStringLiteral("Disconnected"));
    } else {
        msgBox.setText(QStringLiteral("Error: ").append(errorMessage));
        msgBox.setWindowTitle(QStringLiteral("Doh!"));
    }
    msgBox.exec();
    processFinished();
}

void QJackTrip::receivedConnectionFromPeer()
{
    m_ui->statusBar->showMessage(QStringLiteral("Received Connection from Peer!"));
    m_assignedClientName = m_jackTrip->getAssignedClientName();
    if (m_ui->connectScriptCheckBox->isChecked()) {
        QStringList arguments = m_ui->connectScriptEdit->text().split(QStringLiteral(" "),
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                                                                      Qt::SkipEmptyParts);
#else
                                                                      QString::
                                                                          SkipEmptyParts);
#endif
        if (!arguments.isEmpty()) {
            QProcess connectScript;
            connectScript.setProgram(arguments.takeFirst());
            connectScript.setWorkingDirectory(QDir::homePath());
            connectScript.setArguments(arguments);
            connectScript.setStandardOutputFile(QProcess::nullDevice());
            connectScript.setStandardErrorFile(QProcess::nullDevice());
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert(QStringLiteral("JT_CLIENT_NAME"), m_assignedClientName);
            env.insert(QStringLiteral("JT_SEND_CHANNELS"),
                       QString::number(m_ui->channelSendSpinBox->value()));
            env.insert(QStringLiteral("JT_RECV_CHANNELS"),
                       QString::number(m_ui->channelRecvSpinBox->value()));
            connectScript.setProcessEnvironment(env);
            connectScript.startDetached();
        }
    }
}

void QJackTrip::queueLengthChanged(int queueLength)
{
    m_autoQueueIndicator.setText(QStringLiteral("Auto queue: %1").arg(queueLength));
}

void QJackTrip::udpWaitingTooLong()
{
    m_ui->statusBar->showMessage(QStringLiteral("UDP waiting too long (more than 30ms)"),
                                 1000);
}

void QJackTrip::chooseRunType(int index)
{
    // Update ui to reflect choice of run mode.
    if (index == HUB_CLIENT || index == P2P_CLIENT) {
        m_ui->addressComboBox->setEnabled(true);
        m_ui->addressLabel->setEnabled(true);
        if (index == HUB_CLIENT) {
            credentialsChanged();
        } else {
            m_ui->connectButton->setEnabled(
                !m_ui->addressComboBox->currentText().isEmpty());
        }
        m_ui->remotePortSpinBox->setVisible(true);
        m_ui->remotePortLabel->setVisible(true);
        m_ui->connectButton->setText(QStringLiteral("Connect"));
        m_ui->disconnectButton->setText(QStringLiteral("Disconnect"));
    } else {
        m_ui->addressComboBox->setEnabled(false);
        m_ui->addressLabel->setEnabled(false);
        m_ui->remotePortSpinBox->setVisible(false);
        m_ui->remotePortLabel->setVisible(false);
        m_ui->connectButton->setText(QStringLiteral("Start"));
        m_ui->disconnectButton->setText(QStringLiteral("Stop"));
        m_ui->connectButton->setEnabled(true);
    }

    if (index == HUB_SERVER) {
        m_ui->channelGroupBox->setVisible(false);
        m_ui->timeoutCheckBox->setVisible(false);
        m_ui->autoPatchGroupBox->setVisible(true);
        m_ui->requireAuthGroupBox->setVisible(true);
        advancedOptionsForHubServer(true);
        int index = findTab(QStringLiteral("Plugins"));
        if (index != -1) {
            m_ui->optionsTabWidget->removeTab(index);
        }
        index = findTab(QStringLiteral("Scripting"));
        if (index != -1) {
            m_ui->optionsTabWidget->removeTab(index);
        }
        authFilesChanged();
#ifdef RT_AUDIO
        index = findTab(QStringLiteral("Audio Backend"));
        if (index != -1) {
            m_ui->optionsTabWidget->removeTab(index);
        }
#endif
    } else {
        m_ui->autoPatchGroupBox->setVisible(false);
        m_ui->requireAuthGroupBox->setVisible(false);
        m_ui->channelGroupBox->setVisible(true);
        m_ui->timeoutCheckBox->setVisible(true);
        advancedOptionsForHubServer(false);
        if (findTab(QStringLiteral("Plugins")) == -1) {
            m_ui->optionsTabWidget->addTab(m_ui->pluginsTab, QStringLiteral("Plugins"));
        }
        if (findTab(QStringLiteral("Scripting")) == -1) {
            m_ui->optionsTabWidget->addTab(m_ui->scriptingTab,
                                           QStringLiteral("Scripting"));
        }
#ifdef RT_AUDIO
        if (findTab(QStringLiteral("Audio Backend")) == -1) {
            m_ui->optionsTabWidget->insertTab(2, m_ui->backendTab,
                                              QStringLiteral("Audio Backend"));
        }
#endif
    }

    if (index == HUB_CLIENT) {
        m_ui->remoteNameEdit->setVisible(true);
        m_ui->remoteNameLabel->setVisible(true);
        m_ui->authGroupBox->setVisible(true);
    } else {
        m_ui->remoteNameEdit->setVisible(false);
        m_ui->remoteNameLabel->setVisible(false);
        m_ui->authGroupBox->setVisible(false);
    }
}

void QJackTrip::addressChanged(const QString& address)
{
    // Make sure we check that JackTrip isn't running.
    //(This also gets called when we save our recent address list on connecting to a
    // server.)
    if (m_jackTripRunning) {
        return;
    }
    if (m_ui->typeComboBox->currentIndex() == P2P_CLIENT) {
        m_ui->connectButton->setEnabled(!address.isEmpty());
    } else if (m_ui->typeComboBox->currentIndex() == HUB_CLIENT) {
        credentialsChanged();
    }
}

void QJackTrip::authFilesChanged()
{
    if (m_ui->typeComboBox->currentIndex() != HUB_SERVER) {
        return;
    }

    if (m_ui->requireAuthCheckBox->isChecked()
        && (m_ui->certEdit->text().isEmpty() || m_ui->keyEdit->text().isEmpty()
            || m_ui->credsEdit->text().isEmpty())) {
        m_ui->connectButton->setEnabled(false);
    } else {
        m_ui->connectButton->setEnabled(true);
    }
}

void QJackTrip::credentialsChanged()
{
    if (m_ui->typeComboBox->currentIndex() != HUB_CLIENT) {
        return;
    }

    if (m_ui->authCheckBox->isChecked()
        && (m_ui->usernameEdit->text().isEmpty()
            || m_ui->passwordEdit->text().isEmpty())) {
        m_ui->connectButton->setEnabled(false);
    } else {
        m_ui->connectButton->setEnabled(!m_ui->addressComboBox->currentText().isEmpty());
    }
}

void QJackTrip::browseForFile()
{
    QPushButton* sender = static_cast<QPushButton*>(QObject::sender());
    QString fileType;
    QLineEdit* fileEdit;
    if (sender == m_ui->certBrowse) {
        fileType = QStringLiteral("Certificates (*.crt *.pem)");
        fileEdit = m_ui->certEdit;
    } else if (sender == m_ui->keyBrowse) {
        fileType = QStringLiteral("Keys (*.key *.pem)");
        fileEdit = m_ui->keyEdit;
    } else {
        fileType = QLatin1String("");
        if (sender == m_ui->connectScriptBrowse) {
            fileEdit = m_ui->connectScriptEdit;
        } else if (sender == m_ui->disconnectScriptBrowse) {
            fileEdit = m_ui->disconnectScriptEdit;
        } else {
            fileEdit = m_ui->credsEdit;
        }
    }
    QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("Open File"),
                                                    m_lastPath, fileType);
    if (!fileName.isEmpty()) {
        fileEdit->setText(fileName);
        fileEdit->setFocus(Qt::OtherFocusReason);
        m_lastPath = QFileInfo(fileName).canonicalPath();
    }
}

void QJackTrip::receivedIP(QNetworkReply* reply)
{
    QMutexLocker locker(&m_requestMutex);
    m_replyCount++;

    // Check whether we're dealing with our IPv4 or IPv6 request.
    if (reply->url().host().startsWith(QLatin1String("api6"))) {
        if (reply->error() == QNetworkReply::NoError) {
            m_IPv6Address = QString(reply->readAll());
            // Make sure this isn't just a repeat of our IPv4 address.
            if (QHostAddress(m_IPv6Address).protocol() != QAbstractSocket::IPv6Protocol) {
                m_IPv6Address.clear();
                reply->deleteLater();
                return;
            }
        }
    } else {
        if (reply->error() == QNetworkReply::NoError) {
            m_IPv4Address = QString(reply->readAll());
        }
    }

    if (m_replyCount == 2) {
        // Set our label if both replies have arrived.
        if (m_IPv4Address.isEmpty() && m_IPv6Address.isEmpty()) {
            m_ui->ipLabel->setText(
                QStringLiteral("Unable to determine external IP address."));
        } else if (m_IPv4Address.isEmpty()) {
            m_ui->ipLabel->setText(
                QStringLiteral("External IPv6 address: ").append(m_IPv6Address));
            m_ui->ipLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        } else {
            m_ui->ipLabel->setText(
                QStringLiteral("External IP address: ").append(m_IPv4Address));
            m_ui->ipLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
            if (!m_IPv6Address.isEmpty()) {
                m_ui->ipLabel->setText(m_ui->ipLabel->text().append(
                    QStringLiteral("\n(IPv6: %1)").arg(m_IPv6Address)));
            }
        }
    }

    reply->deleteLater();
}

void QJackTrip::resetOptions()
{
    // Reset our basic options
    /*m_ui->channelSpinBox->setValue(2);
    m_ui->autoPatchComboBox->setCurrentIndex(0);
    m_ui->zeroCheckBox->setChecked(false);
    m_ui->timeoutCheckBox->setChecked(false);*/

    // Then advanced options
    m_ui->clientNameEdit->setText(QLatin1String(""));
    m_ui->remoteNameEdit->setText(QLatin1String(""));
    m_ui->localPortSpinBox->setValue(gDefaultPort);
    m_ui->remotePortSpinBox->setValue(gDefaultPort);
    m_ui->basePortSpinBox->setValue(61002);
    m_ui->queueLengthSpinBox->setValue(gDefaultQueueLength);
    m_ui->redundancySpinBox->setValue(gDefaultRedundancy);
    m_ui->resolutionComboBox->setCurrentIndex(1);
    m_ui->connectAudioCheckBox->setChecked(true);
    m_ui->realTimeCheckBox->setChecked(true);
    m_ui->ioStatsCheckBox->setChecked(false);
    m_ui->ioStatsSpinBox->setValue(1);
    m_ui->verboseCheckBox->setChecked(false);

    saveSettings();
}

void QJackTrip::start()
{
    m_ui->connectButton->setEnabled(false);
    enableUi(false);
    m_jackTripRunning = true;

    if (gVerboseFlag) {
        m_debugDialog->show();
    }

    // Start the appropriate JackTrip process.
    try {
        AudioInterface::audioBitResolutionT resolution;
        if (m_ui->resolutionComboBox->currentIndex() == 0) {
            resolution = AudioInterface::BIT8;
        } else if (m_ui->resolutionComboBox->currentIndex() == 1) {
            resolution = AudioInterface::BIT16;
        } else if (m_ui->resolutionComboBox->currentIndex() == 2) {
            resolution = AudioInterface::BIT24;
        } else {
            resolution = AudioInterface::BIT32;
        }

        if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
            m_udpHub.reset(new UdpHubListener(m_ui->localPortSpinBox->value(),
                                              m_ui->basePortSpinBox->value()));
            int hubConnectionMode = hubModeFromPatchType(
                static_cast<patchTypeT>(m_ui->autoPatchComboBox->currentIndex()));
            if (m_ui->patchServerCheckBox->isChecked()) {
                if (m_ui->autoPatchComboBox->currentIndex() == CLIENTFOFI) {
                    hubConnectionMode = JackTrip::SERVFOFI;
                } else if (m_ui->autoPatchComboBox->currentIndex() == FULLMIX) {
                    hubConnectionMode = JackTrip::SERVFULLMIX;
                }
            }

            m_udpHub->setHubPatch(hubConnectionMode);
            m_udpHub->setStereoUpmix(m_ui->upmixCheckBox->isChecked());

            if (m_ui->zeroCheckBox->isChecked()) {
                // Set buffers to zero when underrun
                m_udpHub->setUnderRunMode(JackTrip::ZEROS);
            }
            m_udpHub->setAudioBitResolution(resolution);

            if (!m_ui->jitterCheckBox->isChecked()) {
                m_udpHub->setBufferStrategy(-1);
                m_udpHub->setBufferQueueLength(m_ui->queueLengthSpinBox->value());
            } else {
                m_udpHub->setBufferStrategy(m_ui->bufferStrategyComboBox->currentIndex()
                                            + 1);
                if (m_ui->broadcastCheckBox->isChecked()) {
                    m_udpHub->setBroadcast(m_ui->broadcastQueueSpinBox->value());
                }
                if (m_ui->autoQueueCheckBox->isChecked()) {
                    m_udpHub->setBufferQueueLength(-(m_ui->autoQueueSpinBox->value()));
                    m_autoQueueIndicator.setText(QStringLiteral("Auto queue: enabled"));
                } else {
                    m_udpHub->setBufferQueueLength(m_ui->queueLengthSpinBox->value());
                }
            }
            m_udpHub->setUseRtUdpPriority(m_ui->realTimeCheckBox->isChecked());

            // Enable authentication if needed
            if (m_ui->requireAuthCheckBox->isChecked()) {
                m_udpHub->setRequireAuth(true);
                m_udpHub->setCertFile(m_ui->certEdit->text());
                m_udpHub->setKeyFile(m_ui->keyEdit->text());
                m_udpHub->setCredsFile(m_ui->credsEdit->text());
            }

            // Open our stats window if needed
            if (m_ui->ioStatsCheckBox->isChecked()) {
                m_statsDialog->clearOutput();
                m_statsDialog->show();
                m_udpHub->setIOStatTimeout(m_ui->ioStatsSpinBox->value());
                m_udpHub->setIOStatStream(m_statsDialog->getOutputStream());
            }

            QObject::connect(m_udpHub.data(), &UdpHubListener::signalStopped, this,
                             &QJackTrip::processFinished, Qt::QueuedConnection);
            QObject::connect(m_udpHub.data(), &UdpHubListener::signalError, this,
                             &QJackTrip::processError, Qt::QueuedConnection);
            m_ui->disconnectButton->setEnabled(true);
            m_udpHub->start();
            m_ui->statusBar->showMessage(QStringLiteral("Hub Server Started"));
        } else {
            JackTrip::jacktripModeT jackTripMode;
            if (m_ui->typeComboBox->currentIndex() == P2P_CLIENT) {
                jackTripMode = JackTrip::CLIENT;
            } else if (m_ui->typeComboBox->currentIndex() == P2P_SERVER) {
                jackTripMode = JackTrip::SERVER;
            } else {
                jackTripMode = JackTrip::CLIENTTOPINGSERVER;
            }

            m_jackTrip.reset(new JackTrip(
                jackTripMode, JackTrip::UDP, 0, m_ui->channelSendSpinBox->value(), 0,
                m_ui->channelRecvSpinBox->value(), AudioInterface::MIX_UNSET,
#ifdef WAIR  // wair
                0,
#endif  // endwhere
                m_ui->queueLengthSpinBox->value(), m_ui->redundancySpinBox->value(),
                resolution));
            m_jackTrip->setConnectDefaultAudioPorts(
                m_ui->connectAudioCheckBox->isChecked());
            if (m_ui->zeroCheckBox->isChecked()) {
                // Set buffers to zero when underrun
                m_jackTrip->setUnderRunMode(JackTrip::ZEROS);
            }

#ifdef RT_AUDIO
            if (m_ui->backendComboBox->currentIndex() == 1) {
                unsigned int bufferSize = m_ui->bufferSizeComboBox->currentText().toInt();
                unsigned int sampleRate = m_ui->sampleRateComboBox->currentText().toInt();
                m_jackTrip->setAudiointerfaceMode(JackTrip::RTAUDIO);
                m_jackTrip->setSampleRate(sampleRate);
                m_jackTrip->setAudioBufferSizeInSamples(bufferSize);
                // we assume that first entry is "(default)"
                if (m_ui->inputDeviceComboBox->currentIndex() == 0) {
                    m_jackTrip->setInputDevice("");
                } else {
                    m_jackTrip->setInputDevice(
                        m_ui->inputDeviceComboBox->currentText().toStdString());
                }
                if (m_ui->outputDeviceComboBox->currentIndex() == 0) {
                    m_jackTrip->setOutputDevice("");
                } else {
                    m_jackTrip->setOutputDevice(
                        m_ui->outputDeviceComboBox->currentText().toStdString());
                }
                AudioInterface::setPipewireLatency(bufferSize, sampleRate);
            }
#endif

            if (m_ui->timeoutCheckBox->isChecked()) {
                m_jackTrip->setStopOnTimeout(true);
            }

            if (m_ui->jitterCheckBox->isChecked()) {
                m_jackTrip->setBufferStrategy(m_ui->bufferStrategyComboBox->currentIndex()
                                              + 1);
                if (m_ui->broadcastCheckBox->isChecked()) {
                    m_jackTrip->setBroadcast(m_ui->broadcastQueueSpinBox->value());
                }
                if (m_ui->autoQueueCheckBox->isChecked()) {
                    m_jackTrip->setBufferQueueLength(-(m_ui->autoQueueSpinBox->value()));
                    m_autoQueueIndicator.setText(QStringLiteral("Auto queue: enabled"));
                }
            } else {
                m_jackTrip->setBufferStrategy(-1);
            }
            m_jackTrip->setUseRtUdpPriority(m_ui->realTimeCheckBox->isChecked());

            // Set peer address in client mode
            if (jackTripMode == JackTrip::CLIENT
                || jackTripMode == JackTrip::CLIENTTOPINGSERVER) {
                m_jackTrip->setPeerAddress(
                    m_ui->addressComboBox->currentText().trimmed());
                if (jackTripMode == JackTrip::CLIENTTOPINGSERVER
                    && !m_ui->remoteNameEdit->text().isEmpty()) {
                    m_jackTrip->setRemoteClientName(m_ui->remoteNameEdit->text());
                }
            }

            m_jackTrip->setBindPorts(m_ui->localPortSpinBox->value());
            m_jackTrip->setPeerPorts(m_ui->remotePortSpinBox->value());
            m_jackTrip->setPeerHandshakePort(m_ui->remotePortSpinBox->value());

            if (!m_ui->clientNameEdit->text().isEmpty()) {
                m_jackTrip->setClientName(m_ui->clientNameEdit->text());
            }

            // Set credentials if we're using authentication
            if (m_ui->authCheckBox->isChecked()) {
                m_jackTrip->setUseAuth(true);
                m_jackTrip->setUsername(m_ui->usernameEdit->text());
                m_jackTrip->setPassword(m_ui->passwordEdit->text());
            }

            // Open our stats window if needed
            if (m_ui->ioStatsCheckBox->isChecked()) {
                m_statsDialog->clearOutput();
                m_statsDialog->show();
                m_jackTrip->setIOStatTimeout(m_ui->ioStatsSpinBox->value());
                m_jackTrip->setIOStatStream(m_statsDialog->getOutputStream());
            }

            // Append any plugins
            appendPlugins(m_jackTrip.data(), m_ui->channelSendSpinBox->value(),
                          m_ui->channelRecvSpinBox->value());
            // Setup meters (also currently using faust plugins).
            createMeters(m_ui->channelSendSpinBox->value(),
                         (m_ui->channelRecvSpinBox->value()));

            QObject::connect(m_jackTrip.data(), &JackTrip::signalProcessesStopped, this,
                             &QJackTrip::processFinished, Qt::QueuedConnection);
            QObject::connect(m_jackTrip.data(), &JackTrip::signalError, this,
                             &QJackTrip::processError, Qt::QueuedConnection);
            QObject::connect(
                m_jackTrip.data(), &JackTrip::signalReceivedConnectionFromPeer, this,
                &QJackTrip::receivedConnectionFromPeer, Qt::QueuedConnection);
            QObject::connect(m_jackTrip.data(), &JackTrip::signalUdpWaitingTooLong, this,
                             &QJackTrip::udpWaitingTooLong, Qt::QueuedConnection);
            QObject::connect(m_jackTrip.data(), &JackTrip::signalQueueLengthChanged, this,
                             &QJackTrip::queueLengthChanged, Qt::QueuedConnection);

            m_ui->statusBar->showMessage(QStringLiteral("Waiting for Peer..."));
            m_ui->disconnectButton->setEnabled(true);
#ifdef WAIRTOHUB  // WAIR
            m_jackTrip->startProcess(
                0);  // for WAIR compatibility, ID in jack client name
#else
            m_jackTrip->startProcess();
#endif  // endwhere
        }
    } catch (const std::exception& e) {
        // Let the user know what our exception was.
        QMessageBox msgBox;
        msgBox.setText(QStringLiteral("Error: ").append(e.what()));
        msgBox.setWindowTitle(QStringLiteral("Doh!"));
        msgBox.exec();

        m_jackTripRunning = false;
        enableUi(true);
        m_ui->connectButton->setEnabled(true);
        m_ui->disconnectButton->setEnabled(false);
        m_ui->statusBar->clearMessage();

        return;
    }

    // Add the address to our server history.
    QString serverAddress = m_ui->addressComboBox->currentText().trimmed();
    int serverIndex       = m_ui->addressComboBox->findText(serverAddress);
    if (serverIndex != -1) {
        m_ui->addressComboBox->removeItem(serverIndex);
    }
    m_ui->addressComboBox->insertItem(0, serverAddress);
    m_ui->addressComboBox->setCurrentIndex(0);

    m_interface.disableNap();
}

void QJackTrip::stop()
{
    m_ui->disconnectButton->setEnabled(false);
    if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
        m_udpHub->stop();
    } else {
        m_jackTrip->stop();
    }
}

void QJackTrip::exit()
{
    // Only run this once.
    if (m_isExiting) {
        return;
    }
    m_isExiting = true;
    m_ui->exitButton->setEnabled(false);
    saveSettings();
    if (m_jackTripRunning) {
        stop();
    } else {
        m_exitSent = true;
        emit signalExit();
    }
}

void QJackTrip::updatedInputMeasurements(const float* valuesInDb, int numChannels)
{
    for (int i = 0; i < m_inputMeters.count(); i++) {
        // Determine decibel reading
        qreal dB = m_meterMin;
        if (i < numChannels) {
            dB = std::max(m_meterMin, valuesInDb[i]);
        }

        // Produce a normalized value from 0 to 1
        float level = (dB - m_meterMin) / (m_meterMax - m_meterMin);
        m_inputMeters.at(i)->setLevel(level);
    }
}

void QJackTrip::updatedOutputMeasurements(const float* valuesInDb, int numChannels)
{
    for (int i = 0; i < m_outputMeters.count(); i++) {
        // Determine decibel reading
        qreal dB = m_meterMin;
        if (i < numChannels) {
            dB = std::max(m_meterMin, valuesInDb[i]);
        }

        // Produce a normalized value from 0 to 1
        float level = (dB - m_meterMin) / (m_meterMax - m_meterMin);
        m_outputMeters.at(i)->setLevel(level);
    }
}

#ifndef NO_VS
void QJackTrip::virtualStudioMode()
{
    m_interface.setMode(UserInterface::MODE_VS);
    QSettings settings;
    settings.setValue(QStringLiteral("UiMode"), UserInterface::MODE_VS);
}
#endif

int QJackTrip::findTab(const QString& tabName)
{
    for (int i = 0; i < m_ui->optionsTabWidget->count(); i++) {
        if (m_ui->optionsTabWidget->tabText(i) == tabName) {
            return i;
        }
    }
    return -1;
}

void QJackTrip::enableUi(bool enabled)
{
    if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
        m_ui->optionsTabWidget->setEnabled(enabled);
    } else {
        if (enabled) {
            m_ui->inputGroupBox->setVisible(false);
            m_ui->outputGroupBox->setVisible(false);
            removeMeters();
            m_ui->optionsTabWidget->setVisible(true);
        } else {
            m_ui->optionsTabWidget->setVisible(false);
            m_ui->inputGroupBox->setVisible(true);
            m_ui->outputGroupBox->setVisible(true);
        }
    }
    m_ui->typeLabel->setEnabled(enabled);
    m_ui->typeComboBox->setEnabled(enabled);
    m_ui->addressLabel->setEnabled(
        enabled
        && (m_ui->typeComboBox->currentIndex() == P2P_CLIENT
            || m_ui->typeComboBox->currentIndex() == HUB_CLIENT));
    m_ui->addressComboBox->setEnabled(
        enabled
        && (m_ui->typeComboBox->currentIndex() == P2P_CLIENT
            || m_ui->typeComboBox->currentIndex() == HUB_CLIENT));
}

void QJackTrip::advancedOptionsForHubServer(bool isHubServer)
{
    m_ui->clientNameLabel->setVisible(!isHubServer);
    m_ui->clientNameEdit->setVisible(!isHubServer);
    m_ui->redundancyLabel->setVisible(!isHubServer);
    m_ui->redundancySpinBox->setVisible(!isHubServer);
    m_ui->connectAudioCheckBox->setVisible(!isHubServer);
    m_ui->basePortLabel->setVisible(isHubServer);
    m_ui->basePortSpinBox->setVisible(isHubServer);
    if (isHubServer) {
        m_ui->localPortSpinBox->setToolTip(
            "Set the local TCP port to use for the initial handshake connection. The "
            "default is 4464.");
    } else {
        m_ui->localPortSpinBox->setToolTip(
            "Set the local port to use for the connection. The default is 4464.\n(Useful "
            "for running multiple hub clients behind the same router.)");
    }
}

void QJackTrip::migrateSettings()
{
    // Function to migrate settings for users who previously had QJackTrip installed.
    QSettings settings;
    if (settings.value(QStringLiteral("Migrated"), false).toBool()) {
        return;
    }
#ifdef __APPLE__
    QSettings oldSettings(QStringLiteral("psi-borg.org"), QStringLiteral("QJackTrip"));
#else
    QSettings oldSettings(QStringLiteral("psi-borg"), QStringLiteral("QJackTrip"));
#endif
    QStringList keys = oldSettings.allKeys();
    for (int i = 0; i < keys.size(); i++) {
        settings.setValue(keys.at(i), oldSettings.value(keys.at(i), QVariant()));
    }
    settings.setValue(QStringLiteral("Migrated"), true);
}

void QJackTrip::loadSettings(Settings* cliSettings)
{
    QSettings settings;
    bool useCommandLine = false;
    if (cliSettings) {
        useCommandLine = cliSettings->isModeSet();
    }

    // Migrate to separate send and receive channel numbers first if needed
    int oldChannelSetting = settings.value(QStringLiteral("Channels"), -1).toInt();
    if (oldChannelSetting != -1) {
        settings.setValue(QStringLiteral("ChannelsSend"), oldChannelSetting);
        settings.setValue(QStringLiteral("ChannelsRecv"), oldChannelSetting);
        settings.remove(QStringLiteral("Channels"));
    }

    m_ui->verboseCheckBox->setChecked(
        gVerboseFlag || settings.value(QStringLiteral("Debug"), 0).toBool());
    m_lastPath = settings.value(QStringLiteral("LastPath"), QDir::homePath()).toString();

    settings.beginGroup(QStringLiteral("RecentServers"));
    for (int i = 1; i <= 5; i++) {
        QString address =
            settings.value(QStringLiteral("Server%1").arg(i), "").toString();
        if (!address.isEmpty()) {
            m_ui->addressComboBox->addItem(address);
        }
    }
    settings.endGroup();
    // Need to get this here so it isn't overwritten by the previous section.
    if (useCommandLine && !cliSettings->getPeerAddress().isEmpty()) {
        m_ui->addressComboBox->setCurrentText(cliSettings->getPeerAddress());
    } else {
        m_ui->addressComboBox->setCurrentText(
            settings.value(QStringLiteral("LastAddress"), "").toString());
    }

    if (useCommandLine) {
        JackTrip::jacktripModeT mode = cliSettings->getJackTripMode();
        if (mode == JackTrip::CLIENT) {
            m_ui->typeComboBox->setCurrentIndex(P2P_CLIENT);
        } else if (mode == JackTrip::SERVER) {
            m_ui->typeComboBox->setCurrentIndex(P2P_SERVER);
        } else if (mode == JackTrip::CLIENTTOPINGSERVER) {
            m_ui->typeComboBox->setCurrentIndex(HUB_CLIENT);
        } else {
            m_ui->typeComboBox->setCurrentIndex(HUB_SERVER);
        }
        m_ui->channelSendSpinBox->setValue(cliSettings->getNumAudioInputChans());
        m_ui->channelRecvSpinBox->setValue(cliSettings->getNumAudioOutputChans());

        unsigned int patchMode = cliSettings->getHubConnectionMode();
        if (patchMode == JackTrip::SERVERTOCLIENT) {
            m_ui->autoPatchComboBox->setCurrentIndex(SERVERTOCLIENT);
        } else if (patchMode == JackTrip::CLIENTECHO) {
            m_ui->autoPatchComboBox->setCurrentIndex(CLIENTECHO);
        } else if (patchMode == JackTrip::CLIENTFOFI) {
            m_ui->autoPatchComboBox->setCurrentIndex(CLIENTFOFI);
        } else if (patchMode == JackTrip::FULLMIX) {
            m_ui->autoPatchComboBox->setCurrentIndex(FULLMIX);
        } else {
            // Accomodate for the fact that the GUI doesn't support the reserved patching
            // mode by disabling patching if selected.
            m_ui->autoPatchComboBox->setCurrentIndex(NOAUTO);
        }

        m_ui->patchServerCheckBox->setChecked(cliSettings->getPatchServerAudio());
        m_ui->upmixCheckBox->setChecked(cliSettings->getPatchServerAudio());
        m_ui->zeroCheckBox->setChecked(cliSettings->getUnderrunMode() == JackTrip::ZEROS);
        m_ui->timeoutCheckBox->setChecked(cliSettings->getStopOnTimeout());
        m_ui->clientNameEdit->setText(cliSettings->getClientName());
        m_ui->remoteNameEdit->setText(cliSettings->getRemoteClientName());
        m_ui->localPortSpinBox->setValue(cliSettings->getBindPort());
        m_ui->remotePortSpinBox->setValue(cliSettings->getPeerPort());
        int basePort = cliSettings->getServerUdpPort();
        if (basePort == 0) {
            // TODO: This currently mirrors the behaviour seen in UdpHubListener.cpp, but
            // I'm not sure it's particularly intuitive. It makes sense if the bind port
            // was changed using the offset flag -o but not if it was changed using -B.
            // These two cases are not currently distinguished between.
            basePort = 61002 + cliSettings->getBindPort() - gDefaultPort;
        }
        m_ui->basePortSpinBox->setValue(basePort);
        int queueLength = cliSettings->getQueueLength();
        m_ui->queueLengthSpinBox->setValue(
            queueLength > 0
                ? queueLength
                : settings.value(QStringLiteral("QueueLength"), gDefaultQueueLength)
                      .toInt());
        m_ui->redundancySpinBox->setValue(cliSettings->getRedundancy());
        AudioInterface::audioBitResolutionT resolution =
            cliSettings->getAudioBitResolution();
        if (resolution == AudioInterface::BIT8) {
            m_ui->resolutionComboBox->setCurrentIndex(0);
        } else if (resolution == AudioInterface::BIT16) {
            m_ui->resolutionComboBox->setCurrentIndex(1);
        } else if (resolution == AudioInterface::BIT24) {
            m_ui->resolutionComboBox->setCurrentIndex(2);
        } else {
            m_ui->resolutionComboBox->setCurrentIndex(3);
        }
        m_ui->connectAudioCheckBox->setChecked(
            cliSettings->getConnectDefaultAudioPorts());
        m_ui->realTimeCheckBox->setChecked(cliSettings->getUseRtUdpPriority());

        m_ui->requireAuthCheckBox->setChecked(cliSettings->getUseAuthentication());
        m_ui->authCheckBox->setChecked(cliSettings->getUseAuthentication());
        m_ui->certEdit->setText(cliSettings->getCertFile());
        m_ui->keyEdit->setText(cliSettings->getKeyFile());
        m_ui->credsEdit->setText(cliSettings->getCredsFile());
        m_ui->usernameEdit->setText(cliSettings->getUsername());
        m_ui->passwordEdit->setText(cliSettings->getPassword());

        settings.beginGroup(QStringLiteral("JitterBuffer"));
        settings.setValue(QStringLiteral("JitterAnnounce"), true);
        int bufferStrategy = cliSettings->getBufferStrategy();
        m_ui->jitterCheckBox->setChecked(bufferStrategy > 0);
        m_ui->broadcastCheckBox->setChecked(cliSettings->getBroadCastQueue() > 0);
        m_ui->broadcastQueueSpinBox->setValue(
            cliSettings->getBroadCastQueue() > 0
                ? cliSettings->getBroadCastQueue()
                : settings
                      .value(QStringLiteral("BroadcastLength"), gDefaultQueueLength * 2)
                      .toInt());
        if (bufferStrategy > 0) {
            m_ui->bufferStrategyComboBox->setCurrentIndex(bufferStrategy - 1);
        } else {
            m_ui->bufferStrategyComboBox->setCurrentIndex(
                settings.value(QStringLiteral("Strategy"), 1).toInt() - 1);
        }
        m_ui->autoQueueCheckBox->setChecked(queueLength < 0);
        m_ui->autoQueueSpinBox->setValue(
            queueLength < 0
                ? std::abs(queueLength)
                : settings.value(QStringLiteral("TuningParameter"), 500).toInt());
        settings.endGroup();
    } else {
        m_ui->typeComboBox->setCurrentIndex(
            settings.value(QStringLiteral("RunMode"), 2).toInt());
        m_ui->zeroCheckBox->setChecked(
            settings.value(QStringLiteral("ZeroUnderrun"), false).toBool());
        m_ui->localPortSpinBox->setValue(
            settings.value(QStringLiteral("LocalPort"), gDefaultPort).toInt());
        m_ui->queueLengthSpinBox->setValue(
            settings.value(QStringLiteral("QueueLength"), gDefaultQueueLength).toInt());
        m_ui->resolutionComboBox->setCurrentIndex(
            settings.value(QStringLiteral("Resolution"), 1).toInt());
        m_ui->realTimeCheckBox->setChecked(
            settings.value(QStringLiteral("RTNetworking"), true).toBool());

        settings.beginGroup(QStringLiteral("JitterBuffer"));
        bool jitterAnnounce =
            settings.value(QStringLiteral("JitterAnnounce"), false).toBool();
        if (!jitterAnnounce
            && !settings.value(QStringLiteral("Enabled"), true).toBool()) {
            QMessageBox msgBox;
            msgBox.setText(
                "From this build onwards, the new jitter buffer is being enabled by "
                "default. "
                "You can turn it off in the Jitter Buffer settings tab.");
            msgBox.setWindowTitle(QStringLiteral("Jitter Buffer"));
            msgBox.exec();
            settings.setValue(QStringLiteral("Enabled"), true);
        }
        settings.setValue(QStringLiteral("JitterAnnounce"), true);
        m_ui->jitterCheckBox->setChecked(
            settings.value(QStringLiteral("Enabled"), true).toBool());
        m_ui->broadcastCheckBox->setChecked(
            settings.value(QStringLiteral("Broadcast"), false).toBool());
        m_ui->broadcastQueueSpinBox->setValue(
            settings.value(QStringLiteral("BroadcastLength"), gDefaultQueueLength * 2)
                .toInt());
        m_ui->bufferStrategyComboBox->setCurrentIndex(
            settings.value(QStringLiteral("Strategy"), 1).toInt() - 1);
        m_ui->autoQueueCheckBox->setChecked(
            settings.value(QStringLiteral("AutoQueue"), true).toBool());
        m_ui->autoQueueSpinBox->setValue(
            settings.value(QStringLiteral("TuningParameter"), 500).toInt());
        settings.endGroup();
    }

    // These settings may need to be loaded even if we were using our command line.
    // (This depends on the mode that was selected.)
    if (!useCommandLine || m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
        m_ui->channelSendSpinBox->setValue(
            settings.value(QStringLiteral("ChannelsSend"), gDefaultNumInChannels)
                .toInt());
        m_ui->channelRecvSpinBox->setValue(
            settings.value(QStringLiteral("ChannelsRecv"), gDefaultNumOutChannels)
                .toInt());
        m_ui->timeoutCheckBox->setChecked(
            settings.value(QStringLiteral("Timeout"), false).toBool());
        m_ui->clientNameEdit->setText(
            settings.value(QStringLiteral("ClientName"), "").toString());
        m_ui->redundancySpinBox->setValue(
            settings.value(QStringLiteral("Redundancy"), gDefaultRedundancy).toInt());
        m_ui->connectAudioCheckBox->setChecked(
            settings.value(QStringLiteral("ConnectAudio"), true).toBool());
    }
    if (!useCommandLine || !(m_ui->typeComboBox->currentIndex() == HUB_SERVER)) {
        m_ui->autoPatchComboBox->setCurrentIndex(
            settings.value(QStringLiteral("AutoPatchMode"), 0).toInt());
        m_ui->patchServerCheckBox->setChecked(
            settings.value(QStringLiteral("PatchIncludesServer"), false).toBool());
        m_ui->upmixCheckBox->setChecked(
            settings.value(QStringLiteral("StereoUpmix"), false).toBool());
        m_ui->basePortSpinBox->setValue(
            settings.value(QStringLiteral("BasePort"), 61002).toInt());
    }
    if (!useCommandLine || !(m_ui->typeComboBox->currentIndex() == HUB_CLIENT)) {
        m_ui->remoteNameEdit->setText(
            settings.value(QStringLiteral("RemoteName"), "").toString());
    }
    if (!useCommandLine
        || !(m_ui->typeComboBox->currentIndex() == HUB_CLIENT
             || m_ui->typeComboBox->currentIndex() == P2P_CLIENT)) {
        m_ui->remotePortSpinBox->setValue(
            settings.value(QStringLiteral("RemotePort"), gDefaultPort).toInt());
    }

    settings.beginGroup(QStringLiteral("Auth"));
    if (!useCommandLine || !(m_ui->typeComboBox->currentIndex() == HUB_SERVER)) {
        m_ui->requireAuthCheckBox->setChecked(
            settings.value(QStringLiteral("Require"), false).toBool());
        m_ui->certEdit->setText(
            settings.value(QStringLiteral("CertFile"), "").toString());
        m_ui->keyEdit->setText(settings.value(QStringLiteral("KeyFile"), "").toString());
        m_ui->credsEdit->setText(
            settings.value(QStringLiteral("CredsFile"), "").toString());
    }
    if (!useCommandLine || !(m_ui->typeComboBox->currentIndex() == HUB_CLIENT)) {
        m_ui->authCheckBox->setChecked(
            settings.value(QStringLiteral("Use"), false).toBool());
        m_ui->usernameEdit->setText(
            settings.value(QStringLiteral("Username"), "").toString());
        m_ui->passwordEdit->setText("");
    }
    settings.endGroup();

    // Settings from this point onwards are currently read only from the previously stored
    // values and not from the commmand line.
#ifdef RT_AUDIO
    settings.beginGroup(QStringLiteral("Audio"));
    m_ui->backendComboBox->setCurrentIndex(
        settings.value(QStringLiteral("Backend"), 0).toInt());
    m_ui->sampleRateComboBox->setCurrentText(
        settings.value(QStringLiteral("SampleRate"), "48000").toString());
    m_ui->bufferSizeComboBox->setCurrentText(
        settings.value(QStringLiteral("BufferSize"), "128").toString());
    // update device list and set the device
    populateDeviceMenu(m_ui->inputDeviceComboBox, true);
    auto inDevice = settings.value(QStringLiteral("InputDevice")).toString();
    if (!inDevice.isEmpty()) {
        m_ui->inputDeviceComboBox->setCurrentText(inDevice);
    }
    populateDeviceMenu(m_ui->outputDeviceComboBox, false);
    auto outDevice = settings.value(QStringLiteral("OutputDevice")).toString();
    if (!outDevice.isEmpty()) {
        m_ui->outputDeviceComboBox->setCurrentText(outDevice);
    }
    settings.endGroup();
#endif

    settings.beginGroup(QStringLiteral("IOStats"));
    m_ui->ioStatsCheckBox->setChecked(
        settings.value(QStringLiteral("Display"), false).toBool());
    m_ui->ioStatsSpinBox->setValue(
        settings.value(QStringLiteral("ReportingInterval"), 1).toInt());
    settings.endGroup();

    settings.beginGroup(QStringLiteral("InPlugins"));
    m_ui->inFreeverbCheckBox->setChecked(
        settings.value(QStringLiteral("Freeverb"), false).toBool());
    m_ui->inFreeverbWetnessSlider->setValue(
        settings.value(QStringLiteral("FreeverbWetness"), 0).toInt());
    m_ui->inZitarevCheckBox->setChecked(
        settings.value(QStringLiteral("Zitarev"), false).toBool());
    m_ui->inZitarevWetnessSlider->setValue(
        settings.value(QStringLiteral("ZitarevWetness"), 0).toInt());
    m_ui->inCompressorCheckBox->setChecked(
        settings.value(QStringLiteral("Compressor"), false).toBool());
    m_ui->inLimiterCheckBox->setChecked(
        settings.value(QStringLiteral("Limiter"), false).toBool());
    settings.endGroup();

    settings.beginGroup(QStringLiteral("OutPlugins"));
    m_ui->outFreeverbCheckBox->setChecked(
        settings.value(QStringLiteral("Freeverb"), false).toBool());
    m_ui->outFreeverbWetnessSlider->setValue(
        settings.value(QStringLiteral("FreeverbWetness"), 0).toInt());
    m_ui->outZitarevCheckBox->setChecked(
        settings.value(QStringLiteral("Zitarev"), false).toBool());
    m_ui->outZitarevWetnessSlider->setValue(
        settings.value(QStringLiteral("ZitarevWetness"), 0).toInt());
    m_ui->outCompressorCheckBox->setChecked(
        settings.value(QStringLiteral("Compressor"), false).toBool());
    m_ui->outLimiterCheckBox->setChecked(
        settings.value(QStringLiteral("Limiter"), false).toBool());
    m_ui->outClientsSpinBox->setValue(
        settings.value(QStringLiteral("Clients"), 1).toInt());
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Scripting"));
    m_ui->connectScriptCheckBox->setChecked(
        settings.value(QStringLiteral("ConnectEnabled"), false).toBool());
    m_ui->connectScriptEdit->setText(
        settings.value(QStringLiteral("ConnectScript"), "").toString());
    m_ui->disconnectScriptCheckBox->setChecked(
        settings.value(QStringLiteral("DisconnectEnabled"), false).toBool());
    m_ui->disconnectScriptEdit->setText(
        settings.value(QStringLiteral("DisconnectScript"), "").toString());
    settings.endGroup();
}

void QJackTrip::saveSettings()
{
    QSettings settings;
    settings.setValue(QStringLiteral("RunMode"), m_ui->typeComboBox->currentIndex());
    settings.setValue(QStringLiteral("LastAddress"),
                      m_ui->addressComboBox->currentText());
    settings.setValue(QStringLiteral("ChannelsSend"), m_ui->channelSendSpinBox->value());
    settings.setValue(QStringLiteral("ChannelsRecv"), m_ui->channelRecvSpinBox->value());
    settings.setValue(QStringLiteral("AutoPatchMode"),
                      m_ui->autoPatchComboBox->currentIndex());
    settings.setValue(QStringLiteral("PatchIncludesServer"),
                      m_ui->patchServerCheckBox->isChecked());
    settings.setValue(QStringLiteral("StereoUpmix"), m_ui->upmixCheckBox->isChecked());
    settings.setValue(QStringLiteral("ZeroUnderrun"), m_ui->zeroCheckBox->isChecked());
    settings.setValue(QStringLiteral("Timeout"), m_ui->timeoutCheckBox->isChecked());
    settings.setValue(QStringLiteral("ClientName"), m_ui->clientNameEdit->text());
    settings.setValue(QStringLiteral("RemoteName"), m_ui->remoteNameEdit->text());
    settings.setValue(QStringLiteral("LocalPort"), m_ui->localPortSpinBox->value());
    settings.setValue(QStringLiteral("RemotePort"), m_ui->remotePortSpinBox->value());
    settings.setValue(QStringLiteral("BasePort"), m_ui->basePortSpinBox->value());
    settings.setValue(QStringLiteral("QueueLength"), m_ui->queueLengthSpinBox->value());
    settings.setValue(QStringLiteral("Redundancy"), m_ui->redundancySpinBox->value());
    settings.setValue(QStringLiteral("Resolution"),
                      m_ui->resolutionComboBox->currentIndex());
    settings.setValue(QStringLiteral("ConnectAudio"),
                      m_ui->connectAudioCheckBox->isChecked());
    settings.setValue(QStringLiteral("RTNetworking"),
                      m_ui->realTimeCheckBox->isChecked());
    settings.setValue(QStringLiteral("Debug"), m_ui->verboseCheckBox->isChecked());
    settings.setValue(QStringLiteral("LastPath"), m_lastPath);

    settings.beginGroup(QStringLiteral("RecentServers"));
    for (int i = 0; i < m_ui->addressComboBox->count(); i++) {
        settings.setValue(QStringLiteral("Server%1").arg(i + 1),
                          m_ui->addressComboBox->itemText(i));
    }
    settings.endGroup();

#ifdef RT_AUDIO
    settings.beginGroup(QStringLiteral("Audio"));
    settings.setValue(QStringLiteral("Backend"), m_ui->backendComboBox->currentIndex());
    settings.setValue(QStringLiteral("SampleRate"),
                      m_ui->sampleRateComboBox->currentText());
    settings.setValue(QStringLiteral("BufferSize"),
                      m_ui->bufferSizeComboBox->currentText());
    settings.setValue(QStringLiteral("InputDevice"),
                      m_ui->inputDeviceComboBox->currentText());
    settings.setValue(QStringLiteral("OutputDevice"),
                      m_ui->outputDeviceComboBox->currentText());
    settings.endGroup();
#endif

    settings.beginGroup(QStringLiteral("Auth"));
    settings.setValue(QStringLiteral("Require"), m_ui->requireAuthCheckBox->isChecked());
    settings.setValue(QStringLiteral("CertFile"), m_ui->certEdit->text());
    settings.setValue(QStringLiteral("KeyFile"), m_ui->keyEdit->text());
    settings.setValue(QStringLiteral("CredsFile"), m_ui->credsEdit->text());
    settings.setValue(QStringLiteral("Use"), m_ui->authCheckBox->isChecked());
    settings.setValue(QStringLiteral("Username"), m_ui->usernameEdit->text());
    settings.endGroup();

    settings.beginGroup(QStringLiteral("IOStats"));
    settings.setValue(QStringLiteral("Display"), m_ui->ioStatsCheckBox->isChecked());
    settings.setValue(QStringLiteral("ReportingInterval"), m_ui->ioStatsSpinBox->value());
    settings.endGroup();

    settings.beginGroup(QStringLiteral("JitterBuffer"));
    settings.setValue(QStringLiteral("Enabled"), m_ui->jitterCheckBox->isChecked());
    settings.setValue(QStringLiteral("Broadcast"), m_ui->broadcastCheckBox->isChecked());
    settings.setValue(QStringLiteral("BroadcastLength"),
                      m_ui->broadcastQueueSpinBox->value());
    settings.setValue(QStringLiteral("Strategy"),
                      m_ui->bufferStrategyComboBox->currentIndex() + 1);
    settings.setValue(QStringLiteral("AutoQueue"), m_ui->autoQueueCheckBox->isChecked());
    settings.setValue(QStringLiteral("TuningParameter"), m_ui->autoQueueSpinBox->value());
    settings.endGroup();

    settings.beginGroup(QStringLiteral("InPlugins"));
    settings.setValue(QStringLiteral("Freeverb"), m_ui->inFreeverbCheckBox->isChecked());
    settings.setValue(QStringLiteral("FreeverbWetness"),
                      m_ui->inFreeverbWetnessSlider->value());
    settings.setValue(QStringLiteral("Zitarev"), m_ui->inZitarevCheckBox->isChecked());
    settings.setValue(QStringLiteral("ZitarevWetness"),
                      m_ui->inZitarevWetnessSlider->value());
    settings.setValue(QStringLiteral("Compressor"),
                      m_ui->inCompressorCheckBox->isChecked());
    settings.setValue(QStringLiteral("Limiter"), m_ui->inLimiterCheckBox->isChecked());
    settings.endGroup();

    settings.beginGroup(QStringLiteral("OutPlugins"));
    settings.setValue(QStringLiteral("Freeverb"), m_ui->outFreeverbCheckBox->isChecked());
    settings.setValue(QStringLiteral("FreeverbWetness"),
                      m_ui->outFreeverbWetnessSlider->value());
    settings.setValue(QStringLiteral("Zitarev"), m_ui->outZitarevCheckBox->isChecked());
    settings.setValue(QStringLiteral("ZitarevWetness"),
                      m_ui->outZitarevWetnessSlider->value());
    settings.setValue(QStringLiteral("Compressor"),
                      m_ui->outCompressorCheckBox->isChecked());
    settings.setValue(QStringLiteral("Limiter"), m_ui->outLimiterCheckBox->isChecked());
    settings.setValue(QStringLiteral("Clients"), m_ui->outClientsSpinBox->value());
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Scripting"));
    settings.setValue(QStringLiteral("ConnectEnabled"),
                      m_ui->connectScriptCheckBox->isChecked());
    settings.setValue(QStringLiteral("ConnectScript"), m_ui->connectScriptEdit->text());
    settings.setValue(QStringLiteral("DisconnectEnabled"),
                      m_ui->disconnectScriptCheckBox->isChecked());
    settings.setValue(QStringLiteral("DisconnectScript"),
                      m_ui->disconnectScriptEdit->text());
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Window"));
    settings.setValue(QStringLiteral("Geometry"), saveGeometry());
    settings.endGroup();
}

void QJackTrip::appendPlugins(JackTrip* jackTrip, int numSendChannels,
                              int numRecvChannels)
{
    if (!jackTrip) {
        return;
    }

    // These effects are currently deleted by the AudioInterface of jacktrip.
    // May need to change this code if we move to smart pointers.
    if (m_ui->outCompressorCheckBox->isChecked()) {
        QSharedPointer<ProcessPlugin> pluginPtr(
            new Compressor(numSendChannels, false, CompressorPresets::voice));
        jackTrip->appendProcessPluginToNetwork(pluginPtr);
    }
    if (m_ui->inCompressorCheckBox->isChecked()) {
        QSharedPointer<ProcessPlugin> pluginPtr(
            new Compressor(numRecvChannels, false, CompressorPresets::voice));
        jackTrip->appendProcessPluginFromNetwork(pluginPtr);
    }

    if (m_ui->outZitarevCheckBox->isChecked()) {
        qreal wetness = m_ui->outZitarevWetnessSlider->value() / 100.0;
        QSharedPointer<ProcessPlugin> pluginPtr(
            new Reverb(numSendChannels, numSendChannels, 1.0 + wetness));
        jackTrip->appendProcessPluginToNetwork(pluginPtr);
    }
    if (m_ui->inZitarevCheckBox->isChecked()) {
        qreal wetness = m_ui->inZitarevWetnessSlider->value() / 100.0;
        QSharedPointer<ProcessPlugin> pluginPtr(
            new Reverb(numRecvChannels, numRecvChannels, 1.0 + wetness));
        jackTrip->appendProcessPluginFromNetwork(pluginPtr);
    }

    if (m_ui->outFreeverbCheckBox->isChecked()) {
        qreal wetness = m_ui->outFreeverbWetnessSlider->value() / 100.0;
        QSharedPointer<ProcessPlugin> pluginPtr(
            new Reverb(numSendChannels, numSendChannels, wetness));
        jackTrip->appendProcessPluginToNetwork(pluginPtr);
    }
    if (m_ui->inFreeverbCheckBox->isChecked()) {
        qreal wetness = m_ui->inFreeverbWetnessSlider->value() / 100.0;
        QSharedPointer<ProcessPlugin> pluginPtr(
            new Reverb(numRecvChannels, numRecvChannels, wetness));
        jackTrip->appendProcessPluginFromNetwork(pluginPtr);
    }

    // Limiters go last in the plugin sequence.
    if (m_ui->outLimiterCheckBox->isChecked()) {
        QSharedPointer<ProcessPlugin> pluginPtr(
            new Limiter(numSendChannels, m_ui->outClientsSpinBox->value()));
        jackTrip->appendProcessPluginToNetwork(pluginPtr);
    }
    if (m_ui->inLimiterCheckBox->isChecked()) {
        QSharedPointer<ProcessPlugin> pluginPtr(new Limiter(numRecvChannels, 1));
        jackTrip->appendProcessPluginFromNetwork(pluginPtr);
    }
}

void QJackTrip::createMeters(quint32 inputChannels, quint32 outputChannels)
{
    // These pointers are also deleted by AudioInterface.
    QSharedPointer<ProcessPlugin> inputMeterPtr(new Meter(inputChannels));
    QSharedPointer<ProcessPlugin> outputMeterPtr(new Meter(outputChannels));
    m_jackTrip->appendProcessPluginToNetwork(inputMeterPtr);
    m_jackTrip->appendProcessPluginFromNetwork(outputMeterPtr);

    // Create our widgets.
    for (quint32 i = 0; i < inputChannels; i++) {
        VuMeter* meter = new VuMeter(this);
        m_inputMeters.append(meter);
        QLabel* label = new QLabel(QString::number(i + 1), this);
        m_inputLabels.append(label);
        label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        m_inputLayout->addWidget(label, i, 0, 1, 1);
        m_inputLayout->addWidget(meter, i, 1, 1, 1);
    }
    // Effectively add a spacer at the bottom.
    m_inputLayout->setRowStretch(inputChannels, 100);

    for (quint32 i = 0; i < outputChannels; i++) {
        VuMeter* meter = new VuMeter(this);
        m_outputMeters.append(meter);
        QLabel* label = new QLabel(QString::number(i + 1), this);
        m_outputLabels.append(label);
        label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        m_outputLayout->addWidget(label, i, 0, 1, 1);
        m_outputLayout->addWidget(meter, i, 1, 1, 1);
    }
    m_outputLayout->setRowStretch(outputChannels, 100);

    QObject::connect(static_cast<Meter*>(inputMeterPtr.get()),
                     &Meter::onComputedVolumeMeasurements, this,
                     &QJackTrip::updatedInputMeasurements);
    QObject::connect(static_cast<Meter*>(outputMeterPtr.get()),
                     &Meter::onComputedVolumeMeasurements, this,
                     &QJackTrip::updatedOutputMeasurements);
}

void QJackTrip::removeMeters()
{
    m_inputLayout->setRowStretch(m_inputMeters.count(), 0);
    m_outputLayout->setRowStretch(m_outputMeters.count(), 0);
    for (int i = 0; i < m_inputLabels.count(); i++) {
        delete m_inputLabels.at(i);
    }
    for (int i = 0; i < m_inputMeters.count(); i++) {
        delete m_inputMeters.at(i);
    }
    for (int i = 0; i < m_outputLabels.count(); i++) {
        delete m_outputLabels.at(i);
    }
    for (int i = 0; i < m_outputMeters.count(); i++) {
        delete m_outputMeters.at(i);
    }
    m_inputLabels.clear();
    m_inputMeters.clear();
    m_outputLabels.clear();
    m_outputMeters.clear();
}

QString QJackTrip::commandLineFromCurrentOptions()
{
    QString commandLine = QStringLiteral("jacktrip");

    if (m_ui->typeComboBox->currentIndex() == P2P_CLIENT) {
        commandLine.append(" -c ").append(m_ui->addressComboBox->currentText());
    } else if (m_ui->typeComboBox->currentIndex() == P2P_SERVER) {
        commandLine.append(" -s");
    } else if (m_ui->typeComboBox->currentIndex() == HUB_CLIENT) {
        commandLine.append(" -C ").append(m_ui->addressComboBox->currentText());
    } else {
        commandLine.append(" -S");
    }

    if (m_ui->zeroCheckBox->isChecked()) {
        commandLine.append(" -z");
    }

    if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
        int hubConnectionMode = hubModeFromPatchType(
            static_cast<patchTypeT>(m_ui->autoPatchComboBox->currentIndex()));
        if (hubConnectionMode > 0) {
            commandLine.append(QStringLiteral(" -p %1").arg(hubConnectionMode));
        }
        if (m_ui->patchServerCheckBox->isChecked()
            && (m_ui->typeComboBox->currentIndex() == CLIENTFOFI
                || m_ui->typeComboBox->currentIndex() == FULLMIX)) {
            commandLine.append(" -i");
        }
        if (m_ui->upmixCheckBox->isChecked()) {
            commandLine.append(" -u");
        }
    } else {
        if (m_ui->channelSendSpinBox->value() != gDefaultNumInChannels
            || m_ui->channelRecvSpinBox->value() != gDefaultNumOutChannels) {
            if (m_ui->channelSendSpinBox->value() == m_ui->channelRecvSpinBox->value()) {
                commandLine.append(
                    QStringLiteral(" -n %1").arg(m_ui->channelRecvSpinBox->value()));
            } else {
                commandLine.append(
                    QStringLiteral(" --receivechannels %1 --sendchannels %2")
                        .arg(m_ui->channelRecvSpinBox->value())
                        .arg(m_ui->channelSendSpinBox->value()));
            }
        }
        if (m_ui->timeoutCheckBox->isChecked()) {
            commandLine.append(" -t");
        }
    }

    int bufStrategy = -1;
    if (m_ui->jitterCheckBox->isChecked()) {
        bufStrategy = m_ui->bufferStrategyComboBox->currentIndex() + 1;
    }
    if (bufStrategy != 1) {
        commandLine.append(QStringLiteral(" --bufstrategy %1").arg(bufStrategy));
    }

    if (m_ui->jitterCheckBox->isChecked() && m_ui->autoQueueCheckBox->isChecked()) {
        if (m_ui->autoQueueSpinBox->value() == 500) {
            commandLine.append(" -q auto");
        } else {
            commandLine.append(
                QStringLiteral(" -q auto%1").arg(m_ui->autoQueueSpinBox->value()));
        }
    } else if (m_ui->queueLengthSpinBox->value() != gDefaultQueueLength) {
        commandLine.append(
            QStringLiteral(" -q %1").arg(m_ui->queueLengthSpinBox->value()));
    }

    if (m_ui->jitterCheckBox->isChecked() && m_ui->broadcastCheckBox->isChecked()) {
        commandLine.append(
            QStringLiteral(" --broadcast %1").arg(m_ui->broadcastQueueSpinBox->value()));
    }

    // Port settings
    if (m_ui->localPortSpinBox->value() != gDefaultPort) {
        commandLine.append(QStringLiteral(" -B %1").arg(m_ui->localPortSpinBox->value()));
    }
    if (m_ui->typeComboBox->currentIndex() == HUB_CLIENT
        || m_ui->typeComboBox->currentIndex() == P2P_CLIENT) {
        if (m_ui->remotePortSpinBox->value() != gDefaultPort) {
            commandLine.append(
                QStringLiteral(" -P %1").arg(m_ui->remotePortSpinBox->value()));
        }
    }

    // Auth settings
    if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
        if (m_ui->requireAuthCheckBox->isChecked()) {
            commandLine.append(" -A");
            if (!m_ui->certEdit->text().isEmpty()) {
                commandLine.append(" --certfile ").append(m_ui->certEdit->text());
            }
            if (!m_ui->keyEdit->text().isEmpty()) {
                commandLine.append(" --keyfile ").append(m_ui->keyEdit->text());
            }
            if (!m_ui->credsEdit->text().isEmpty()) {
                commandLine.append(" --credsfile ").append(m_ui->credsEdit->text());
            }
        }
    } else if (m_ui->typeComboBox->currentIndex() == HUB_CLIENT) {
        if (m_ui->authCheckBox->isChecked()) {
            commandLine.append(" -A");
            if (!m_ui->usernameEdit->text().isEmpty()) {
                commandLine.append(" --username ").append(m_ui->usernameEdit->text());
            }
            /*if (!m_ui->passwordEdit->text().isEmpty()) {
                commandLine.append(" --password <password>");
            }*/
        }
    }

    if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
        int offset = m_ui->localPortSpinBox->value() - gDefaultPort;
        if (m_ui->basePortSpinBox->value() != 61002 + offset) {
            commandLine.append(
                QStringLiteral(" -U %1").arg(m_ui->basePortSpinBox->value()));
        }
    } else {
        if (!m_ui->clientNameEdit->text().isEmpty()) {
            commandLine.append(
                QStringLiteral(" -J \"%1\"").arg(m_ui->clientNameEdit->text()));
        }
        if (m_ui->typeComboBox->currentIndex() == HUB_CLIENT
            && !m_ui->remoteNameEdit->text().isEmpty()) {
            commandLine.append(
                QStringLiteral(" -K \"%1\"").arg(m_ui->remoteNameEdit->text()));
        }
        if (m_ui->redundancySpinBox->value() > 1) {
            commandLine.append(
                QStringLiteral(" -r %1").arg(m_ui->redundancySpinBox->value()));
        }
        if (m_ui->resolutionComboBox->currentText() != QLatin1String("16")) {
            commandLine.append(" -b ").append(m_ui->resolutionComboBox->currentText());
        }
        if (!m_ui->connectAudioCheckBox->isChecked()) {
            commandLine.append(" -D");
        }

        if (m_ui->inLimiterCheckBox->isChecked()
            || m_ui->outLimiterCheckBox->isChecked()) {
            commandLine.append(" -O ");
            if (m_ui->inLimiterCheckBox->isChecked()) {
                commandLine.append("i");
            }
            if (m_ui->outLimiterCheckBox->isChecked()) {
                commandLine.append("o");
                if (m_ui->outClientsSpinBox->value() != 2) {
                    commandLine.append(
                        QStringLiteral(" -a %1").arg(m_ui->outClientsSpinBox->value()));
                }
            }
        }

        bool inEffects = m_ui->inFreeverbCheckBox->isChecked()
                         || m_ui->inZitarevCheckBox->isChecked()
                         || m_ui->inCompressorCheckBox->isChecked();
        bool outEffects = m_ui->outFreeverbCheckBox->isChecked()
                          || m_ui->outZitarevCheckBox->isChecked()
                          || m_ui->outCompressorCheckBox->isChecked();
        if (inEffects || outEffects) {
            commandLine.append(" -f \"");
            if (inEffects) {
                commandLine.append("i:");
                if (m_ui->inCompressorCheckBox->isChecked()) {
                    commandLine.append("c");
                }
                if (m_ui->inFreeverbCheckBox->isChecked()) {
                    commandLine.append(QStringLiteral("f(%1)").arg(
                        m_ui->inFreeverbWetnessSlider->value() / 100.0));
                }
                if (m_ui->inZitarevCheckBox->isChecked()) {
                    commandLine.append(QStringLiteral("f(%1)").arg(
                        m_ui->inZitarevWetnessSlider->value() / 100.0));
                }
                if (outEffects) {
                    commandLine.append(", ");
                }
            }
            if (outEffects) {
                commandLine.append("o:");
                if (m_ui->outCompressorCheckBox->isChecked()) {
                    commandLine.append("c");
                }
                if (m_ui->outFreeverbCheckBox->isChecked()) {
                    commandLine.append(QStringLiteral("f(%1)").arg(
                        m_ui->outFreeverbWetnessSlider->value() / 100.0));
                }
                if (m_ui->outZitarevCheckBox->isChecked()) {
                    commandLine.append(QStringLiteral("f(%1)").arg(
                        m_ui->outZitarevWetnessSlider->value() / 100.0));
                }
            }
            commandLine.append("\"");
        }
    }
    if (m_ui->ioStatsCheckBox->isChecked()) {
        commandLine.append(QStringLiteral(" -I %1").arg(m_ui->ioStatsSpinBox->value()));
    }

    if (m_ui->verboseCheckBox->isChecked()) {
        commandLine.append(" -V");
    }

    if (m_ui->realTimeCheckBox->isChecked()) {
        commandLine.append(" --udprt");
    }

#ifdef RT_AUDIO
    if (m_ui->typeComboBox->currentIndex() != HUB_SERVER
        && m_ui->backendComboBox->currentIndex() == 1) {
        commandLine.append(" --rtaudio");
        commandLine.append(
            QStringLiteral(" --srate %1").arg(m_ui->sampleRateComboBox->currentText()));
        commandLine.append(
            QStringLiteral(" --bufsize %1").arg(m_ui->bufferSizeComboBox->currentText()));
        QString inDevice;
        if (m_ui->inputDeviceComboBox->currentIndex() > 0) {
            inDevice = m_ui->inputDeviceComboBox->currentText();
        }
        QString outDevice;
        if (m_ui->outputDeviceComboBox->currentIndex() > 0) {
            outDevice = m_ui->outputDeviceComboBox->currentText();
        }
        QString inDeviceEscaped =
            QString(inDevice).replace(QStringLiteral(","), QStringLiteral("\\,"));
        QString outDeviceEscaped =
            QString(outDevice).replace(QStringLiteral(","), QStringLiteral("\\,"));
        commandLine.append(QStringLiteral(" --audiodevice \"%1\",\"%2\"")
                               .arg(inDeviceEscaped, outDeviceEscaped));
    }
#endif

    return commandLine;
}

#ifdef RT_AUDIO
void QJackTrip::populateDeviceMenu(QComboBox* menu, bool isInput)
{
    QString previousString = menu->currentText();
    menu->clear();
    menu->addItem(QStringLiteral("(default)"));

    QVector<RtAudioDevice> devices;
    RtAudioInterface::scanDevices(devices);
    for (auto info : devices) {
        // convert names to QString to gracefully handle invalid
        // utf8 character sequences, such as "RØDE Microphone"
        const QString utf8Name(QString::fromStdString(info.name));

        // Don't include duplicate entries
        if (menu->findText(utf8Name) != -1) {
            continue;
        }
        if (isInput && info.inputChannels > 0) {
            menu->addItem(utf8Name);
        } else if (!isInput && info.outputChannels > 0) {
            menu->addItem(utf8Name);
        }
    }

    // set the previous value
    menu->setCurrentText(previousString);
}
#endif

void QJackTrip::showCommandLineMessageBox()
{
    QMessageBox msgBox;
    QString messageText =
        QStringLiteral("The equivalent command line for the current options is:\n\n%1")
            .arg(commandLineFromCurrentOptions());
    msgBox.setText(messageText);
    msgBox.setWindowTitle(QStringLiteral("Command Line"));
    msgBox.setTextInteractionFlags(Qt::TextSelectableByMouse);
    msgBox.exec();
}

JackTrip::hubConnectionModeT QJackTrip::hubModeFromPatchType(
    QJackTrip::patchTypeT patchType)
{
    if (patchType == SERVERTOCLIENT) {
        return JackTrip::SERVERTOCLIENT;
    } else if (patchType == CLIENTECHO) {
        return JackTrip::CLIENTECHO;
    } else if (patchType == CLIENTFOFI) {
        return JackTrip::CLIENTFOFI;
    } else if (patchType == FULLMIX) {
        return JackTrip::FULLMIX;
    } else {
        return JackTrip::NOAUTO;
    }
}

QJackTrip::~QJackTrip()
{
    // Restore cout. (Stops a crash on exit.)
    std::cout.rdbuf(m_realCout.rdbuf());
    std::cerr.rdbuf(m_realCerr.rdbuf());
}

QCoreApplication* QJackTrip::createApplication(int& argc, char* argv[])
{
    // Turn on high DPI support.
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    // Fix for display scaling like 125% or 150% on Windows
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#if defined(NO_VS) && defined(_WIN32)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
#else
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif  // NO_VS
#endif  // QT_VERSION

    return new QApplication(argc, argv);
}
