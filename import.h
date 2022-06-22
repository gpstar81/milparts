//      import.h
//      Copyright 2010 Michael Rajotte <michael@michaelrajotte.com>
// 		Imports the inventory database from a csv file.

#define MAXFLDS 200     /* the maximum possible number of fields */
#define MAXFLDSIZE 256   /* the longest possible field + 1 = 31 byte field */

typedef struct _ImportData {
	GtkWidget		*mainWindow;
	GtkWidget		*importContainer, *importTree;
	GtkTreeSelection *selection;
	
	GtkWidget		*importButton, *deleteButton;

	int fieldCounter;
	int recordCounter;
	
	char tmp[1024];
	char dataArray[MAXFLDS][MAXFLDSIZE];
	
	/* Boolean flags for selected column id properties for importation */
	gboolean idFlag, partNoFlag, descriptionFlag, manufacturerFlag, replacementFlag, costFlag, costAvgFlag, priceFlag, stockFlag, lowStockLvlFlag, lastSoldFlag;
	
} ImportData;

int importDatabase(GtkWidget *);
static int checkIfExist(gchar *);
static int importData(gchar *, GtkWidget *);
static int sendData(gchar *);
static void columnClicked(GtkWidget *, ImportData *);

static void setFlagsFalse(ImportData *, gchar *);
static void setFlagsTrue(GtkWidget *, ImportData *);

static void freeMemory(GtkWidget *, ImportData *);
static void destroyWindow(GtkWidget *, GtkWidget *);

static void parseCSV(char *record, char *delim, char dataArray[][MAXFLDSIZE], int *fieldCounter);
static void removeRows(GtkWidget *, ImportData *);
static void beginRowRemoval(GtkTreeRowReference *, ImportData *);

static void importRows(GtkWidget *, ImportData *);
static void beginImport(GtkTreeRowReference *, ImportData *);

static gboolean treeButtonPress(GtkWidget *, GdkEventButton *, ImportData *);
static gboolean keyPressSelection(gpointer);
static gboolean treeKeyPress(GtkWidget *, GdkEventKey *, ImportData *);
