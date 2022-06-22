//      inventory_cat.h
//      Copyright 2010-2014 Michael Rajotte <michael@michaelrajotte.com>
// 		Creating item categories and adding items to them.

enum {
	ID,
	CATNAME,
	PARENT,
	ORDER,
	SECTION,
	SECTION_ID,
	CAT_COLUMNS
};

enum {
	CAT_ID,
	CAT_BARCODE,
	CAT_DESCRIPTION,
	CAT_MANUFACTURER,
	CAT_EXTRAINFO,
	CAT_INVENTORY_COLUMNS
};

/* Not needed */
/*
enum {
	BARCODE,
	NAME,
	DESCRIPTION,
	MANUFACTURER,
	CATEGORY,
	COST,
	COSTAVG,
	PRICE,
	STOCK,
	INVENTORY_COLUMNS
};
*/

void initalizeCategories(GtkWidget *);

static void freeMemoryCatWindow(GtkWidget *, intrackCategories *);
static void closeWindow(GtkWidget *, intrackCategories *);
static void prepareViewWindow(GtkWidget *, intrackCategories *);
static void prepareLoadItemAdd(GtkWidget *, intrackCategories *);

static void catFreeMemory(GtkWidget *, intrackCategories *);
static void catAddSensitive(GtkWidget *, intrackCategories *);

static void setupCategoryTree(intrackCategories *);
static void setupCatInventoryTree(intrackCategories *);
static void getCategories(GtkWidget *, intrackCategories *);
static void createCatWindow(GtkWidget *, intrackCategories *);
static void destroyWidget(GtkWidget *, gpointer);
static void setupInventoryTree(intrackCategories *);
//static void getInventory(GtkWidget *, intrackCategories *);
//static void getInventoryNumbers(GtkWidget *, intrackCategories *);

static int prepareItemRemove(GtkWidget *, intrackCategories *);
static void beginItemRemove(GtkTreeRowReference *, intrackCategories *);
static int removeItemsWindow(GtkWidget *, intrackCategories *);

int inventoryModifyCategory(gchar *, gchar *, gchar*);
int getCatInventory(GtkWidget *, gchar *);

static int deleteCatWindow(GtkWidget *, intrackCategories *);
static int checkCatExist(gchar *);
static int pullCategories(GtkTreeStore *);
static int databaseCreateCat(GtkWidget *, intrackCatEntry *);
static int databaseRemoveCat(gchar *);
//static int pullInventory(GtkListStore *, gchar *, gchar *, gchar *, gchar *);
static int cellClickedCat(GtkCellRendererText *, gchar *, gchar *, intrackCategories *);
static void cellClickedParent(GtkCellRendererText *, gchar *, gchar *, intrackCategories *);
static void cellClickedSection(GtkCellRendererText *, gchar *, gchar *, intrackCategories *);
static int categoryUpdateName(gchar *, gchar *);
int categoryUpdate(gchar *); // -> category_popup.c calls this.


static int databaseCategoriesRemove(gchar *, gchar *, gchar *);
static int databaseCategoriesUpdate(gchar *, gchar *, int, gchar *, gchar *, gchar *);
static int databaseCategoriesEdit(gchar *, gchar *, gchar *, gchar *);

static void clear_tree_store(intrackCategories *);

//static float getAverageCost(gchar *);

static gboolean cat_button_press(GtkWidget *, GdkEventButton *, intrackCategories *);
static gboolean cat_key_press(GtkWidget *, GdkEventKey *, intrackCategories *);
static gboolean treeview_key_timer(gpointer);
static gboolean treeButtonPress(GtkWidget *, GdkEventButton *, intrackCategories *);
