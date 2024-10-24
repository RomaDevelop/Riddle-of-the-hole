#include "logedclass.h"

Logs::MsgWorker LogsSt::cbfLogSt = nullptr;
Logs::MsgWorker LogsSt::cbfWarningSt = nullptr;
Logs::MsgWorker LogsSt::cbfErrorSt = nullptr;
QTextBrowser * LogsSt::tBrowserSt = nullptr;
QString LogsSt::msgPrefixSt_ = "";
