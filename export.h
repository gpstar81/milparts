//      export.h
//      Copyright 2010-2014 Michael Rajotte <michael@michaelrajotte.com>
// 		Exports the inventory database to a .csv file.

int mysqlExportSales(GtkWidget *, gpointer, gchar *);
int mysqlExportDatabase(GtkWidget *, gpointer, gchar *);
int mysqlExportCASL(GtkWidget *, gpointer, gchar *);

static int exportTables(char *, gchar *);
static int exportTablesSales(char *, gchar *);
static void importTables(char *);
static void mysqlImportDatabase(GtkWidget *, gpointer);
