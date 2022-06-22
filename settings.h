//      settings.h
//      Copyright 2011 Michael Rajotte <michael@michaelrajotte.com>
// 		Settings and configuration settings.

MYSQL *connection, mysql;
MYSQL_RES *result;
MYSQL_ROW row;

// Setup configuration parameters.
char *mysqlServer;
char *mysqlUsername;
char *mysqlPassword;
char *mysqlDatabase;
char *mysqlTables;

GtkWidget  *window;  // --> This is the main program window widget with most of the widgets inside it.
GtkStatusIcon *tray_icon; // --> System Tray Icon.

GtkWidget	*configMYSQLServer, *configMYSQLUsername, *configMYSQLPassword, *configMYSQLDatabase;


