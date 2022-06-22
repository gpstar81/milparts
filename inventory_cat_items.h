//      inventory_cat_items.h
//      Copyright 2010-2014 Michael Rajotte <michael@michaelrajotte.com>
// 		Adding items to categories.

MYSQL *connection, mysql;
MYSQL_RES *result;
MYSQL_ROW row;

enum {
	ID,
	BARCODE,
	DESCRIPTION,
	MANUFACTURER,
	CATEGORY,
	COST,
	COSTAVG,
	PRICE,
	STOCK,
	INVENTORY_COLUMNS
};

void categoryAddItems(intrackCategories *);
static void destroyWindow(GtkWidget *, intrackCategories *);
static void freeMemory(GtkWidget *, intrackCategories *);
static void setupTree(ItemContainer *);
static void prepareViewWindow(GtkWidget *, intrackCategories *);

static int prepareItemAdd(GtkWidget *, intrackCategories *);
static void beginItemAdd(GtkTreeRowReference *, intrackCategories *);
static void clear_tree_store(intrackCategories *);

static int checkIfExist(gchar *);

static void getInventory(GtkWidget *, intrackCategories *);
static void getInventoryNumbers(GtkWidget *, intrackCategories *);

static int pullInventory(GtkListStore *, gchar *, gchar *, gchar *, gchar *);
static float getAverageCost(gchar *);

static gboolean treeButtonPress(GtkWidget *, GdkEventButton *, intrackCategories *);
static gboolean treeKeyPress(GtkWidget *, GdkEventKey *, intrackCategories *);
static gboolean keyPressSelection(gpointer);
