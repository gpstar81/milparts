//      systems_add.h
//      Copyright 2010-2011 Michael Rajotte <michael@michaelrajotte.com>
// 		Manually adding a system item to the database.

//MYSQL *connection, mysql;
//MYSQL_RES *result;
//MYSQL_ROW row;

typedef struct _IntrackAdd {
	/* The main widget window for the item editor */
	GtkWidget	*mainWindow;
	
	//GtkWidget	*itemImage;
	GtkWidget	*addButton, *cancelButton;
	//GtkWidget	*imageLabel, *imageButton;
	//GtkWidget	*catLabel;
	GtkWidget	*catEntry, *serial1Entry, *serial2Entry, *serial3Entry, *dateInEntry, *dateOutEntry, *soldToEntry, *invoiceEntry;
		
}IntrackAdd;


void initalizeSystemAdd(GtkWidget *);

static void destroyWindow(GtkWidget *, GtkWidget *);
static void freeMemory(GtkWidget *, IntrackAdd *);

static void prepareLoadCat(GtkWidget *, IntrackAdd *);
static void imageChooserDialog(GtkWidget *, IntrackAdd *);
static void sendData(GtkWidget *, IntrackAdd *);

static int createData(IntrackAdd *);
static int checkIfExist(gchar *);
static int databaseQuery(gchar *, gchar *);
