/***************************************************************************
 *   Copyright (C) 2006 by Vladimir Kuznetsov                              *
 *   vovanec@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include <QApplication>
#include <QtGlobal>

#ifdef _WIN32
    #include "win32/windows_compat.h"
#endif

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <utility>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <direct.h>
    #define chdir _chdir
    // Define getopt structures and constants for Windows
    struct option {
        const char *name;
        int has_arg;
        int *flag;
        int val;
    };
    #define no_argument 0
    #define required_argument 1
    #define optional_argument 2
    
    // Windows getopt implementation
    char *optarg = nullptr;
    int optind = 1;
    int opterr = 1;
    int optopt = 0;
    
    int getopt_long(int argc, char *const argv[], const char *optstring,
                    const struct option *longopts, int *longindex);
#else
    #include <getopt.h>
    #include <unistd.h>
#endif

#ifdef HAVE_QDBUS
    #include <QtDBus/QtDBus>
    #include "processadaptor.h"
#endif


#include "mainwindow.h"
#include "qterminalapp.h"
#include "qterminalutils.h"
#include "terminalconfig.h"

#define out

const char* const short_options = "vhw:e:dp:";

static const char* serviceName = "org.lxqt.QTerminal";
static const char* ifaceName = "org.lxqt.QTerminal.Process";

const struct option long_options[] = {
    {"version", 0, nullptr, 'v'},
    {"help",    0, nullptr, 'h'},
    {"workdir", 1, nullptr, 'w'},
    {"execute", 1, nullptr, 'e'},
    {"drop",    0, nullptr, 'd'},
    {"profile", 1, nullptr, 'p'},
    {nullptr,   0, nullptr,  0}
};

QTerminalApp * QTerminalApp::m_instance = nullptr;

[[ noreturn ]] void print_usage_and_exit(int code)
{
    printf("QTerminal %s\n", QTERMINAL_VERSION);
    puts("Usage: qterminal [OPTION]...\n");
    puts("  -d,  --drop               Start in \"dropdown mode\" (like Yakuake or Tilda)");
    puts("  -e,  --execute <command>  Execute command instead of shell");
    puts("  -h,  --help               Print this help");
    puts("  -p,  --profile <name>     Load profile from ~/.config/<name>.conf");
    puts("  -v,  --version            Prints application version and exits");
    puts("  -w,  --workdir <dir>      Start session with specified work directory");
    puts("\nHomepage: <https://github.com/lxqt/qterminal>");
    puts("Report bugs to <https://github.com/lxqt/qterminal/issues>");
    exit(code);
}

[[ noreturn ]] void print_version_and_exit(int code=0)
{
    printf("%s\n", QTERMINAL_VERSION);
    exit(code);
}

void parse_args(int argc, char* argv[], QString& workdir, QStringList & shell_command, out bool& dropMode)
{
    int next_option = 0;
    dropMode = false;
    do{
        next_option = getopt_long(argc, argv, short_options, long_options, nullptr);
        switch(next_option)
        {
            case 'h':
                print_usage_and_exit(0);
                break;
            case 'w':
                workdir = QString::fromLocal8Bit(optarg);
                break;
            case 'e':
                shell_command << parse_command(QString::fromLocal8Bit(optarg));
                // #15 "Raw" -e params
                // Passing "raw" params (like konsole -e mcedit /tmp/tmp.txt") is more preferable - then I can call QString("qterminal -e ") + cmd_line in other programs
                while (optind < argc)
                {
                    //printf("arg: %d - %s\n", optind, argv[optind]);
                    shell_command << QString::fromLocal8Bit(argv[optind++]);
                }
                break;
            case 'd':
                dropMode = true;
                break;
            case 'p':
                Properties::Instance(QString::fromLocal8Bit(optarg));
                break;
            case '?':
                print_usage_and_exit(1);
                break;
            case 'v':
                print_version_and_exit();
                break;
        }
    }
    while(next_option != -1);

    // FIXME: The app might not exit in the dropdown mode after the shell command is terminated
    // and the window is closed. For now, the dropdown mode is disabled with command execution.
    if (!shell_command.isEmpty())
    {
        dropMode = false;
    }
}

int main(int argc, char *argv[])
{
    if (!qEnvironmentVariableIsEmpty("XPC_SERVICE_NAME")) {
        // On macOS, if qterminal.app is spawned by launchd (e.g., from Finder
        // or use `open qterminal.app`, $PWD is set to /. Workaround that by
        // go to $HOME first.
        if (chdir(QDir::homePath().toLatin1().data())) {
#ifdef _WIN32
            qDebug() << "Failed to chdir to $HOME" << QDir::homePath() << "Error code:" << errno;
#else
            qDebug() << "Failed to chdir to $HOME" << QDir::homePath() << strerror(errno);
#endif
        }

        // also initializes $LANG
        QString systemLocaleName(QLocale().name());
        systemLocaleName.append(QLatin1String(".UTF-8"));
        qputenv("LANG", systemLocaleName.toLatin1());
    }

    QApplication::setApplicationName(QStringLiteral("qterminal"));
    QApplication::setApplicationVersion(QStringLiteral(QTERMINAL_VERSION));
    QApplication::setOrganizationDomain(QStringLiteral("qterminal.org"));
    QApplication::setDesktopFileName(QLatin1String("qterminal"));
    // Warning: do not change settings format. It can screw bookmarks later.
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QTerminalApp *app = QTerminalApp::Instance(argc, argv);

    QString workdir;
    QStringList shell_command;
    bool dropMode = false;
    parse_args(argc, argv, workdir, shell_command, dropMode);

    #ifdef HAVE_QDBUS
        app->registerOnDbus(dropMode);
    #endif

    if (!app->isPrimaryInstance())
    {
        app->requestDropDown();
        return 0;
    }

    Properties::Instance()->migrate_settings();
    Properties::Instance()->loadSettings();

    if (workdir.isEmpty())
        workdir = QDir::currentPath();
    app->setWorkingDirectory(workdir);

    const QSettings settings;
    const QFileInfo customStyle = QFileInfo(
        QFileInfo(settings.fileName()).canonicalPath() +
        QStringLiteral("/style.qss")
    );
    if (customStyle.isFile() && customStyle.isReadable())
    {
        QFile style(customStyle.canonicalFilePath());
        style.open(QFile::ReadOnly);
        QString styleString = QLatin1String(style.readAll());
        app->setStyleSheet(styleString);
    }

    // icons
    /* setup our custom icon theme if there is no system theme (OS X, Windows) */
    if (QIcon::themeName().isEmpty())
        QIcon::setThemeName(QStringLiteral("QTerminal"));

    // translations

    // install the translations built-into Qt itself
    QTranslator qtTranslator;
    if (qtTranslator.load(QStringLiteral("qt_") + QLocale::system().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
    {
        app->installTranslator(&qtTranslator);
    }

    QTranslator translator;
    bool installTr = false;
    QString fname = QString::fromLatin1("qterminal_%1.qm").arg(QLocale::system().name().left(5));
#ifdef TRANSLATIONS_DIR
    //qDebug() << "TRANSLATIONS_DIR: Loading translation file" << fname << "from dir" << TRANSLATIONS_DIR;
    installTr = translator.load(fname, QString::fromUtf8(TRANSLATIONS_DIR), QStringLiteral("_"));
#endif
#ifdef APPLE_BUNDLE
    QDir translations_dir = QDir(QApplication::applicationDirPath());
    translations_dir.cdUp();
    if (translations_dir.cd(QStringLiteral("Resources/translations"))) {
        installTr = translator.load(fname, translations_dir.path(), QStringLiteral("_"));
    } /*else {
        qWarning() << "Unable to find \"Resources/translations\" dir in" << translations_dir.path();
    }*/
#endif
    if (installTr)
    {
        app->installTranslator(&translator);
    }

    TerminalConfig initConfig = TerminalConfig(workdir, shell_command);
    app->newWindow(dropMode, initConfig);

    int ret = app->exec();
    delete Properties::Instance();
    app->cleanup();

    return ret;
}

MainWindow *QTerminalApp::newWindow(bool dropMode, TerminalConfig &cfg)
{
    MainWindow *window = nullptr;
    if (dropMode)
    {
        window = new MainWindow(cfg, dropMode);
        if (Properties::Instance()->dropShowOnStart)
            window->show();
    }
    else
    {
        window = new MainWindow(cfg, dropMode);
        if (Properties::Instance()->saveSizeOnExit
            && Properties::Instance()->windowMaximized)
        {
            window->setWindowState(Qt::WindowMaximized);
        }
        window->show();
    }
    return window;
}

QTerminalApp *QTerminalApp::Instance()
{
    assert(m_instance != nullptr);
    return m_instance;
}

QTerminalApp *QTerminalApp::Instance(int &argc, char **argv)
{
    assert(m_instance == nullptr);
    m_instance = new QTerminalApp(argc, argv);
    return m_instance;
}

QTerminalApp::QTerminalApp(int &argc, char **argv)
    :QApplication(argc, argv)
{
}

QString &QTerminalApp::getWorkingDirectory()
{
    return m_workDir;
}

void QTerminalApp::setWorkingDirectory(const QString &wd)
{
    m_workDir = wd;
}

void QTerminalApp::cleanup() {
    delete m_instance;
    m_instance = nullptr;
}


void QTerminalApp::addWindow(MainWindow *window)
{
    m_windowList.append(window);
}

void QTerminalApp::removeWindow(MainWindow *window)
{
    m_windowList.removeOne(window);
}

QList<MainWindow *> QTerminalApp::getWindowList()
{
    return m_windowList;
}

#ifdef HAVE_QDBUS
void QTerminalApp::registerOnDbus(bool dropDown)
{
    if (!QDBusConnection::sessionBus().isConnected())
    {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
        return;
    }

    if (dropDown)
    {
        if (!QDBusConnection::sessionBus().registerService(QLatin1String(serviceName)))
        {
            m_isPrimaryInstance = false;
            return;
        }
        new ProcessAdaptor(this);
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/"), this);
    }
    else
    {
        if (!QDBusConnection::sessionBus().registerService(QLatin1String(serviceName)
                                                           + QStringLiteral("-%1").arg(getpid())))
        {
            fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
            return;
        }
        new ProcessAdaptor(this);
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/"), this);
    }
}

QList<QDBusObjectPath> QTerminalApp::getWindows()
{
    QList<QDBusObjectPath> windows;
    for (MainWindow *wnd : std::as_const(m_windowList))
    {
        windows.push_back(wnd->getDbusPath());
    }
    return windows;
}

QDBusObjectPath QTerminalApp::newWindow(const QHash<QString,QVariant> &termArgs)
{
    TerminalConfig cfg = TerminalConfig::fromDbus(termArgs);
    MainWindow *wnd = newWindow(false, cfg);
    assert(wnd != nullptr);
    return wnd->getDbusPath();
}

QDBusObjectPath QTerminalApp::getActiveWindow()
{
    QWidget *aw = activeWindow();
    if (aw == nullptr)
        return QDBusObjectPath("/");
    return qobject_cast<MainWindow*>(aw)->getDbusPath();
}

bool QTerminalApp::isDropMode() {
  if (m_windowList.count() == 0) {
    return false;
  }
  MainWindow *wnd = m_windowList.at(0);
  return wnd->dropMode();
}

bool QTerminalApp::toggleDropdown() {
  if (m_windowList.count() == 0) {
    return false;
  }
  MainWindow *wnd = m_windowList.at(0);
  if (!wnd->dropMode()) {
    return false;
  }
  wnd->showHide();
  return true;
}

void QTerminalApp::requestDropDown()
{
    QDBusInterface iface(QLatin1String(serviceName),
                         QStringLiteral("/"),
                         QLatin1String(ifaceName), QDBusConnection::sessionBus(), this);
    iface.call(QStringLiteral("toggleDropdown"));
}

bool QTerminalApp::isPrimaryInstance() {
  return m_isPrimaryInstance;
}


#endif

#ifdef _WIN32
// Windows getopt implementation
int getopt_long(int argc, char *const argv[], const char *optstring,
                const struct option *longopts, int *longindex) {
    static int next_char = 0;
    static char *current_arg = nullptr;
    
    if (optind >= argc || argv[optind] == nullptr) {
        return -1;
    }
    
    if (current_arg == nullptr) {
        current_arg = argv[optind];
        next_char = 0;
        
        if (current_arg[0] != '-') {
            return -1;
        }
        
        if (current_arg[1] == '-') {
            // Long option
            if (current_arg[2] == '\0') {
                // End of options
                optind++;
                return -1;
            }
            
            char *option_name = current_arg + 2;
            char *equals = strchr(option_name, '=');
            
            if (longopts != nullptr) {
                for (int i = 0; longopts[i].name != nullptr; i++) {
                    size_t name_len = strlen(longopts[i].name);
                    if (equals != nullptr) {
                        if (strncmp(option_name, longopts[i].name, equals - option_name) == 0 &&
                            name_len == (size_t)(equals - option_name)) {
                            if (longopts[i].has_arg == no_argument) {
                                return -1; // Error: option doesn't take argument
                            }
                            optarg = equals + 1;
                            optind++;
                            current_arg = nullptr;
                            return longopts[i].val;
                        }
                    } else {
                        if (strcmp(option_name, longopts[i].name) == 0) {
                            if (longopts[i].has_arg == required_argument) {
                                if (optind + 1 >= argc) {
                                    return -1; // Error: option requires argument
                                }
                                optarg = argv[optind + 1];
                                optind += 2;
                            } else {
                                optarg = nullptr;
                                optind++;
                            }
                            current_arg = nullptr;
                            return longopts[i].val;
                        }
                    }
                }
            }
            return -1; // Unknown long option
        } else {
            next_char = 1;
        }
    }
    
    // Short option
    char c = current_arg[next_char];
    if (c == '\0') {
        optind++;
        current_arg = nullptr;
        return getopt_long(argc, argv, optstring, longopts, longindex);
    }
    
    next_char++;
    
    char *option_char = strchr(optstring, c);
    if (option_char == nullptr) {
        if (current_arg[next_char] == '\0') {
            optind++;
            current_arg = nullptr;
        }
        return '?';
    }
    
    if (option_char[1] == ':') {
        // Option takes argument
        if (current_arg[next_char] != '\0') {
            optarg = current_arg + next_char;
            optind++;
            current_arg = nullptr;
        } else {
            if (optind + 1 >= argc) {
                optind++;
                current_arg = nullptr;
                return '?';
            }
            optarg = argv[optind + 1];
            optind += 2;
            current_arg = nullptr;
        }
    } else {
        optarg = nullptr;
        if (current_arg[next_char] == '\0') {
            optind++;
            current_arg = nullptr;
        }
    }
    
    return c;
}
#endif

