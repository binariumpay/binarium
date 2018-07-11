// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include "config/binarium-config.h"
#endif

#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include "bitcoinunits.h"
#include "guiutil.h"
#include "optionsmodel.h"

#include "intro.h"

#include "validation.h" // for DEFAULT_SCRIPTCHECK_THREADS and MAX_SCRIPTCHECK_THREADS
#include "netbase.h"
#include "txdb.h" // for -dbcache defaults

#include "miner.h"

#ifdef ENABLE_WALLET
#include "wallet/wallet.h" // for CWallet::GetRequiredFee()

#include "privatesend-client.h"
#endif // ENABLE_WALLET

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include <QDataWidgetMapper>
#include <QDir>
#include <QIntValidator>
#include <QLocale>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include <QSettings>

#ifdef ENABLE_WALLET
extern CWallet* pwalletMain;
#endif // ENABLE_WALLET



extern bool g_bGenerateBlocks;



OptionsDialog::OptionsDialog(QWidget *parent, bool enableWallet) :
    QDialog(parent),
    ui(new Ui::OptionsDialog),
    model(0),
    mapper(0)
{
    ui->setupUi(this);

    /* Main elements init */
    ui->databaseCache->setMinimum(nMinDbCache);
    ui->databaseCache->setMaximum(nMaxDbCache);
    ui->threadsScriptVerif->setMinimum(-GetNumCores());
    ui->threadsScriptVerif->setMaximum(MAX_SCRIPTCHECK_THREADS);

    /* Network elements init */
#ifndef USE_UPNP
    ui->mapPortUpnp->setEnabled(false);
#endif

    ui->proxyIp->setEnabled(false);
    ui->proxyPort->setEnabled(false);
    ui->proxyPort->setValidator(new QIntValidator(1, 65535, this));

    ui->proxyIpTor->setEnabled(false);
    ui->proxyPortTor->setEnabled(false);
    ui->proxyPortTor->setValidator(new QIntValidator(1, 65535, this));

    connect(ui->connectSocks, SIGNAL(toggled(bool)), ui->proxyIp, SLOT(setEnabled(bool)));
    connect(ui->connectSocks, SIGNAL(toggled(bool)), ui->proxyPort, SLOT(setEnabled(bool)));
    connect(ui->connectSocks, SIGNAL(toggled(bool)), this, SLOT(updateProxyValidationState()));

    connect(ui->connectSocksTor, SIGNAL(toggled(bool)), ui->proxyIpTor, SLOT(setEnabled(bool)));
    connect(ui->connectSocksTor, SIGNAL(toggled(bool)), ui->proxyPortTor, SLOT(setEnabled(bool)));
    connect(ui->connectSocksTor, SIGNAL(toggled(bool)), this, SLOT(updateProxyValidationState()));

    /* Window elements init */
#ifdef Q_OS_MAC
    /* remove Window tab on Mac */
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabWindow));
#endif

    /* remove Wallet tab in case of -disablewallet */
    if (!enableWallet) {
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabWallet));
    }

    /* Display elements init */
    
    /* Number of displayed decimal digits selector */
    QString digits;
    for(int index = 2; index <=8; index++){
        digits.setNum(index);
        ui->digits->addItem(digits, digits);
    }
    
    /* Theme selector */
    //ui->theme->addItem(QString("BINARIUM-traditional"), QVariant("trad"));
    ui->theme->addItem(QString("BINARIUM-light"), QVariant("light"));
    //ui->theme->addItem(QString("BINARIUM-light-hires"), QVariant("light-hires"));
    ui->theme->addItem(QString("BINARIUM-blue"), QVariant("drkblue"));
    ui->theme->addItem(QString("BINARIUM-Crownium"), QVariant("crownium"));

    /* Language selector */
    QDir translations(":translations");
    ui->lang->addItem(QString("(") + tr("default") + QString(")"), QVariant(""));
    Q_FOREACH(const QString &langStr, translations.entryList())
    {
        QLocale locale(langStr);

        /** check if the locale name consists of 2 parts (language_country) */
        if(langStr.contains("_"))
        {
#if QT_VERSION >= 0x040800
            /** display language strings as "native language - native country (locale name)", e.g. "Deutsch - Deutschland (de)" */
            ui->lang->addItem(locale.nativeLanguageName() + QString(" - ") + locale.nativeCountryName() + QString(" (") + langStr + QString(")"), QVariant(langStr));
#else
            /** display language strings as "language - country (locale name)", e.g. "German - Germany (de)" */
            ui->lang->addItem(QLocale::languageToString(locale.language()) + QString(" - ") + QLocale::countryToString(locale.country()) + QString(" (") + langStr + QString(")"), QVariant(langStr));
#endif
        }
        else
        {
#if QT_VERSION >= 0x040800
            /** display language strings as "native language (locale name)", e.g. "Deutsch (de)" */
            ui->lang->addItem(locale.nativeLanguageName() + QString(" (") + langStr + QString(")"), QVariant(langStr));
#else
            /** display language strings as "language (locale name)", e.g. "German (de)" */
            ui->lang->addItem(QLocale::languageToString(locale.language()) + QString(" (") + langStr + QString(")"), QVariant(langStr));
#endif
        }
    }
#if QT_VERSION >= 0x040700
    ui->thirdPartyTxUrls->setPlaceholderText("https://example.com/tx/%s");
#endif

    ui->unit->setModel(new BitcoinUnits(this));

    /* Widget-to-option mapper */
    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->setOrientation(Qt::Vertical);

    /* setup/change UI elements when proxy IPs are invalid/valid */
    ui->proxyIp->setCheckValidator(new ProxyAddressValidator(parent));
    ui->proxyIpTor->setCheckValidator(new ProxyAddressValidator(parent));
    connect(ui->proxyIp, SIGNAL(validationDidChange(QValidatedLineEdit *)), this, SLOT(updateProxyValidationState()));
    connect(ui->proxyIpTor, SIGNAL(validationDidChange(QValidatedLineEdit *)), this, SLOT(updateProxyValidationState()));
    connect(ui->proxyPort, SIGNAL(textChanged(const QString&)), this, SLOT(updateProxyValidationState()));
    connect(ui->proxyPortTor, SIGNAL(textChanged(const QString&)), this, SLOT(updateProxyValidationState()));



}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::setModel(OptionsModel *model)
{
    QSettings settings;
    QString sDataDir;

    this->model = model;

    if(model)
    {
        /* check if client restart is needed and show persistent message */
        if (model->isRestartRequired())
            showRestartWarning(true);

        QString strLabel = model->getOverriddenByCommandLine();
        if (strLabel.isEmpty())
            strLabel = tr("none");
        ui->overriddenByCommandLineLabel->setText(strLabel);

        //ui -> edDataDir -> setText ( model -> sDataDir );

        mapper->setModel(model);
        setMapper();
        mapper->toFirst();

        updateDefaultProxyNets();

        /*//ui -> edDataDir -> setText ( model -> sDataDir );
        g_bGenerateBlocks = settings.value ( "bGenerateBlocks" ).toBool ();  // model -> data ( model -> bGenerateBlocks, Qt::EditRole ).toBool ()
        fprintf(stdout, "OptionsDialog.setModel () : bGenerateBlocks = %s.\n", settings.value ( "bGenerateBlocks" ).toString ().toUtf8 ().data () );
        GenerateBitcoins ( g_bGenerateBlocks, GetArg ( "-genproclimit", DEFAULT_GENERATE_THREADS ), Params (), * g_connman );*/

    }

    /* warn when one of the following settings changes by user action (placed here so init via mapper doesn't trigger them) */

    /* Main */
    connect(ui->databaseCache, SIGNAL(valueChanged(int)), this, SLOT(showRestartWarning()));
    connect(ui->threadsScriptVerif, SIGNAL(valueChanged(int)), this, SLOT(showRestartWarning()));
    connect(ui->cbEnableMining, SIGNAL(clicked(bool)), this, SLOT(showRestartWarning()));
    connect(ui->spinboxAmountOfMiningThreads, SIGNAL(valueChanged(int)), this, SLOT(showRestartWarning()));
    /* Wallet */
    connect(ui->showMasternodesTab, SIGNAL(clicked(bool)), this, SLOT(showRestartWarning()));
    connect(ui->spendZeroConfChange, SIGNAL(clicked(bool)), this, SLOT(showRestartWarning()));
    /* Network */
    connect(ui->allowIncoming, SIGNAL(clicked(bool)), this, SLOT(showRestartWarning()));
    connect(ui->connectSocks, SIGNAL(clicked(bool)), this, SLOT(showRestartWarning()));
    connect(ui->connectSocksTor, SIGNAL(clicked(bool)), this, SLOT(showRestartWarning()));
    /* Display */
    connect(ui->digits, SIGNAL(valueChanged()), this, SLOT(showRestartWarning()));
    connect(ui->theme, SIGNAL(valueChanged()), this, SLOT(showRestartWarning()));
    connect(ui->lang, SIGNAL(valueChanged()), this, SLOT(showRestartWarning()));
    connect(ui->thirdPartyTxUrls, SIGNAL(textChanged(const QString &)), this, SLOT(showRestartWarning()));
    
    connect(ui->btDataDir, SIGNAL(released()), this, SLOT(btDataDirClicked()));  // clicked(bool)
    connect(ui -> edDataDir, SIGNAL(editingFinished()), this, SLOT(edDataDir_editingFinished()));
    connect(ui -> edDataDir, SIGNAL(textEdited(const QString &)), this, SLOT(edDataDir_textEdited(const QString &)));

    //settings.setValue("strDataDir", "/media/rodion/Data1/ZoneDriverTesting/Binarium/Data/Linux/");

    /*if (settings.contains("strDataDir"))
        sDataDir = settings.value("strDataDir", "").toString();    

    //QByteArray array = model -> sDataDir.toUtf8();
    QByteArray array = sDataDir.toUtf8();
    fprintf(stdout, "OptionsDialog.setModel () : sDataDir = %s.\n", array.data ());  // strUsage.c_str()

    ui -> edDataDir -> setText ( sDataDir );

    //fprintf(stdout, "OptionsDialog.setModel () : Data directory = %s.\n", _( "Data directory" ).c_str () );
    //fprintf(stdout, "OptionsDialog.setModel () : Options = %s.\n", _( "Options" ).c_str () );
    //ui -> lbDataDirectory -> setText ( QString::fromStdString( _( "Data directory" ) ) );

    bool bGenerateBlocks = settings.value ( "bGenerateBlocks", true ).toBool ();
    ui -> cbEnableMining -> setChecked ( bGenerateBlocks );*/

}

void OptionsDialog::setMapper()
{
    /* Main */
    mapper->addMapping(ui->bitcoinAtStartup, OptionsModel::StartAtStartup);
    mapper->addMapping(ui->threadsScriptVerif, OptionsModel::ThreadsScriptVerif);
    mapper->addMapping(ui->databaseCache, OptionsModel::DatabaseCache);

    /* Wallet */
    mapper->addMapping(ui->coinControlFeatures, OptionsModel::CoinControlFeatures);
    mapper->addMapping(ui->showMasternodesTab, OptionsModel::ShowMasternodesTab);
    mapper->addMapping(ui->showAdvancedPSUI, OptionsModel::ShowAdvancedPSUI);
    mapper->addMapping(ui->lowKeysWarning, OptionsModel::LowKeysWarning);
    mapper->addMapping(ui->privateSendMultiSession, OptionsModel::PrivateSendMultiSession);
    mapper->addMapping(ui->spendZeroConfChange, OptionsModel::SpendZeroConfChange);
    mapper->addMapping(ui->privateSendRounds, OptionsModel::PrivateSendRounds);
    mapper->addMapping(ui->privateSendAmount, OptionsModel::PrivateSendAmount);

    /* Network */
    mapper->addMapping(ui->mapPortUpnp, OptionsModel::MapPortUPnP);
    mapper->addMapping(ui->allowIncoming, OptionsModel::Listen);

    mapper->addMapping(ui->connectSocks, OptionsModel::ProxyUse);
    mapper->addMapping(ui->proxyIp, OptionsModel::ProxyIP);
    mapper->addMapping(ui->proxyPort, OptionsModel::ProxyPort);

    mapper->addMapping(ui->connectSocksTor, OptionsModel::ProxyUseTor);
    mapper->addMapping(ui->proxyIpTor, OptionsModel::ProxyIPTor);
    mapper->addMapping(ui->proxyPortTor, OptionsModel::ProxyPortTor);

    /* Window */
#ifndef Q_OS_MAC
    mapper->addMapping(ui->hideTrayIcon, OptionsModel::HideTrayIcon);
    mapper->addMapping(ui->minimizeToTray, OptionsModel::MinimizeToTray);
    mapper->addMapping(ui->minimizeOnClose, OptionsModel::MinimizeOnClose);
#endif

    /* Display */
    mapper->addMapping(ui->digits, OptionsModel::Digits);
    mapper->addMapping(ui->theme, OptionsModel::Theme);
    mapper->addMapping(ui->lang, OptionsModel::Language);
    mapper->addMapping(ui->unit, OptionsModel::DisplayUnit);
    mapper->addMapping(ui->thirdPartyTxUrls, OptionsModel::ThirdPartyTxUrls);

    mapper->addMapping(ui->edDataDir, OptionsModel::sDataDir);
    mapper->addMapping(ui->cbEnableMining, OptionsModel::bGenerateBlocks);
    mapper->addMapping(ui->spinboxAmountOfMiningThreads, OptionsModel::iAmountOfMiningThreads);

    mapper->addMapping(ui->cbEnableMiningInPool, OptionsModel::bEnableMiningInPool);
    mapper->addMapping(ui->spinboxAmountOfPoolMiningThreads, OptionsModel::iAmountOfPoolMiningThreads);
    mapper->addMapping(ui->edPoolURL, OptionsModel::sPoolURL);
    mapper->addMapping(ui->edUser, OptionsModel::sPoolUser);
    mapper->addMapping(ui->edPassword, OptionsModel::sPoolUserPassword);
    mapper->addMapping(ui->edPoolMiningAlgorithm, OptionsModel::sPoolMiningAlgorithm);
    mapper->addMapping(ui->spinboxPoolMinerCPUPriority, OptionsModel::iPoolMinerCPUPriority);
    mapper->addMapping(ui->spinboxPoolMinerCPUAffinity, OptionsModel::iPoolMinerCPUAffinity);

    mapper->addMapping(ui->cbConfirmQuit, OptionsModel::bConfirmQuit);

}

void OptionsDialog::setOkButtonState(bool fState)
{
    ui->okButton->setEnabled(fState);
}

void OptionsDialog::on_resetButton_clicked()
{
    if(model)
    {
        // confirmation dialog
        QMessageBox::StandardButton btnRetVal = QMessageBox::question(this, tr("Confirm options reset"),
            tr("Client restart required to activate changes.") + "<br><br>" + tr("Client will be shut down. Do you want to proceed?"),
            QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

        if(btnRetVal == QMessageBox::Cancel)
            return;

        /* reset all options and close GUI */
        model->Reset();
        QApplication::quit();
    }
}

void OptionsDialog::on_okButton_clicked()
{
    QSettings settings;

    /*if ( ! model -> sDataDir.isEmpty () ) {
        settings.setValue("strDataDir", model -> sDataDir);
    }

    fprintf(stdout, "OptionsDialog.on_okButton_clicked () : ui -> cbEnableMining -> isChecked () = %i.\n", ui -> cbEnableMining -> isChecked () );
    settings.setValue ( "bGenerateBlocks", ui -> cbEnableMining -> isChecked () );
    fprintf(stdout, "OptionsDialog.on_okButton_clicked () : bGenerateBlocks = %s.\n", settings.value ( "bGenerateBlocks" ).toString ().toUtf8 ().data () );*/

    mapper->submit();
#ifdef ENABLE_WALLET
    privateSendClient.nCachedNumBlocks = std::numeric_limits<int>::max();
    if(pwalletMain)
        pwalletMain->MarkDirty();
#endif // ENABLE_WALLET
    accept();
    updateDefaultProxyNets();
}

void OptionsDialog::on_cancelButton_clicked()
{
    reject();
}

void OptionsDialog::on_hideTrayIcon_stateChanged(int fState)
{
    if(fState)
    {
        ui->minimizeToTray->setChecked(false);
        ui->minimizeToTray->setEnabled(false);
    }
    else
    {
        ui->minimizeToTray->setEnabled(true);
    }
}

void OptionsDialog::showRestartWarning(bool fPersistent)
{
    ui->statusLabel->setStyleSheet("QLabel { color: red; }");

    if(fPersistent)
    {
        ui->statusLabel->setText(tr("Client restart required to activate changes."));
    }
    else
    {
        ui->statusLabel->setText(tr("This change would require a client restart."));
        // clear non-persistent status label after 10 seconds
        // Todo: should perhaps be a class attribute, if we extend the use of statusLabel
        QTimer::singleShot(10000, this, SLOT(clearStatusLabel()));
    }
}

void OptionsDialog::btDataDirClicked () { // bool fPersistent
    //QSettings settings;

    /*Intro::pickDataDirectory( true );

    fprintf(stdout, "OptionsDialog.btDataDirClicked () : Function is called : %s.\n", settings.value("strDataDir", "").toString());  // strUsage.c_str()

    ui -> lbDataDir -> setText ( QString::fromStdString( GetDataDir().string() ) ); // GetDataDir().string() settings.value("strDataDirDefault").toString()
    ui -> edDataDir -> setText ( QString::fromStdString( GetDataDir().string() ) ); // QString::fromStdString( GetDataDir( true ).string() )*/

    QString dir = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(0, "Choose data directory", ui -> edDataDir -> text () ) );
    if ( ! dir.isEmpty () ) {
        //model -> sDataDir = dir;
        ui -> edDataDir -> setText ( dir );
        //settings.setValue("strDataDir", dir);
    } //-if
}

void OptionsDialog::edDataDir_editingFinished () {
    if ( !boost::filesystem::exists ( GUIUtil::qstringToBoostPath ( ui -> edDataDir -> text () ) ) ) {
        ui -> lbDataDirMessage -> setText ( "Given directory does not exist." );
    } else ui -> lbDataDirMessage -> setText ( "" );
}

void OptionsDialog::edDataDir_textEdited ( const QString &text ) {
    QByteArray array = text.toUtf8();
    fprintf(stdout, "OptionsDialog.edDataDir_textEdited () : text = %s.\n", array.data ());

   if ( !boost::filesystem::exists ( GUIUtil::qstringToBoostPath ( text ) ) ) {
        ui -> lbDataDirMessage -> setText ( "Given directory does not exist." );
    } else ui -> lbDataDirMessage -> setText ( "" );
}

void OptionsDialog::clearStatusLabel()
{
    ui->statusLabel->clear();
}

void OptionsDialog::updateProxyValidationState()
{
    QValidatedLineEdit *pUiProxyIp = ui->proxyIp;
    QValidatedLineEdit *otherProxyWidget = (pUiProxyIp == ui->proxyIpTor) ? ui->proxyIp : ui->proxyIpTor;
    if (pUiProxyIp->isValid() && (!ui->proxyPort->isEnabled() || ui->proxyPort->text().toInt() > 0) && (!ui->proxyPortTor->isEnabled() || ui->proxyPortTor->text().toInt() > 0))
    {
        setOkButtonState(otherProxyWidget->isValid()); //only enable ok button if both proxys are valid
        ui->statusLabel->clear();
    }
    else
    {
        setOkButtonState(false);
        ui->statusLabel->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel->setText(tr("The supplied proxy address is invalid."));
    }
}

void OptionsDialog::updateDefaultProxyNets()
{
    proxyType proxy;
    std::string strProxy;
    QString strDefaultProxyGUI;

    GetProxy(NET_IPV4, proxy);
    strProxy = proxy.proxy.ToStringIP() + ":" + proxy.proxy.ToStringPort();
    strDefaultProxyGUI = ui->proxyIp->text() + ":" + ui->proxyPort->text();
    (strProxy == strDefaultProxyGUI.toStdString()) ? ui->proxyReachIPv4->setChecked(true) : ui->proxyReachIPv4->setChecked(false);

    GetProxy(NET_IPV6, proxy);
    strProxy = proxy.proxy.ToStringIP() + ":" + proxy.proxy.ToStringPort();
    strDefaultProxyGUI = ui->proxyIp->text() + ":" + ui->proxyPort->text();
    (strProxy == strDefaultProxyGUI.toStdString()) ? ui->proxyReachIPv6->setChecked(true) : ui->proxyReachIPv6->setChecked(false);

    GetProxy(NET_TOR, proxy);
    strProxy = proxy.proxy.ToStringIP() + ":" + proxy.proxy.ToStringPort();
    strDefaultProxyGUI = ui->proxyIp->text() + ":" + ui->proxyPort->text();
    (strProxy == strDefaultProxyGUI.toStdString()) ? ui->proxyReachTor->setChecked(true) : ui->proxyReachTor->setChecked(false);
}

ProxyAddressValidator::ProxyAddressValidator(QObject *parent) :
QValidator(parent)
{
}

QValidator::State ProxyAddressValidator::validate(QString &input, int &pos) const
{
    Q_UNUSED(pos);
    // Validate the proxy
    CService serv(LookupNumeric(input.toStdString().c_str(), 9050));
    proxyType addrProxy = proxyType(serv, true);
    if (addrProxy.IsValid())
        return QValidator::Acceptable;

    return QValidator::Invalid;
}