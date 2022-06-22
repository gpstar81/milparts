//      inventory.h
//      Copyright 2010 - 2011 - 2012 - 2013 - 2014 Michael Rajotte <michael@michaelrajotte.com>
// 		For inventory control, mass listing, searching and editing via a gtktree.


typedef struct _intrackInventory {
	GtkWidget	*inventoryTree, *inventoryViewport;
	GtkTreeSelection *selection;

	GtkWidget	*inventorySearchEntry;
	GtkWidget	*barcodeSearch, *descriptionSearch, *categorySearch, *replaceSearch;
	GtkWidget	*costSearch, *priceSearch, *stockSearch;
	GtkWidget	*searchSpinMin, *searchSpinMax;
	
	//GtkWidget	*invenMenu, *invenMenuView, *invenMenuRemove, *invenMenuEdit;
	GtkWidget	*invenMenu, *invenMenuRemove, *invenMenuView, *invenMenuLink;
	GtkWidget	*invenCategoryButton;
	
	/* One of the Toolbar buttons. Sensitvity changes on this. Thats why it is in the struct. */
	//GtkWidget	*invenViewItemButton, *removeItemButton, *editItemButton; 
	GtkWidget	*removeItemButton;
	
	GtkWidget	*numberOfPartsLabel, *partCostsLabel, *partCostsAvgLabel;
	GtkWidget	*ytdItemsLabel, *ytdSalesLabel, *ytdEstSalesLabel;
	
	//GtkWidget 	*part_button, *desc_button, *manu_button, *replace_button, *weight_button, *disc_button, *cost_button, *costavg_button, *costavgttl_button, *profit_button, *price_button, *dealer_button, *stock_button, *order_button, *totalsold_button, *totalsoldamount_button, *lastsold_button, *extrainfo_button, *category_button, *length_button, *width_button, *height_button, *margin_button, *ytd_button, *est_button, *ytd_amount_button, *est_amount_button;
	
	gchar 		*mysqlDatabase;
	gchar		*inventoryTable;
	gchar		*selectedItemCode;
	gchar		*exportQueryString;
	
	GtkWidget	*mainWindow; // The main program widget window
	
	GtkWidget	*hide_button;
	int int_part_button, int_desc_button, int_extra_button, int_manu_button, int_replace_button, int_weight_button, int_disc_button, int_cost_button, int_costavgttl_button, int_profit_button, int_price_button, int_dealer_button, int_order_button, int_stock_button, int_costavg_button, int_totalsold_button, int_totalsoldamount_button, int_lastsold_button, int_category_button, int_length_button, int_width_button, int_height_button, int_margin_button, int_ytd_button, int_est_button, int_ytd_amount_button, int_est_amount_button;

}intrackInventory;

typedef struct _AddItem {
	GtkWidget	*addItemCode, *addItemName, *addItemDescription, *addItemStockQty, *addItemPrice, *addItemCost, *addItemLabel;
	GtkWidget	*addItemImage; // --> This is the main image placeholder for drawing images in the data viewport screen in additem mode.
	GtkWidget	*clearDataButton;
	GtkWidget	*addItemSendButton, *addItemImageButton, *addItemCheckButton, *addItemSerialQuantity, *addItemSerialLabel;
	GtkWidget	*addItemViewport;
} AddItem;

typedef struct _EditItem {
	GtkWidget	*editItemImage, *editItemImageUpload;
	GtkWidget	*editItemName, *editItemDescription, *editItemStockQty, *editItemUpdateButton, *editItemLabel, *editItemPrice, *editItemCost;
	GtkWidget	*editItemScanEntry;
	GtkWidget	*editItemViewport, *editItemClearButton;
	GtkWidget	*editItemCheckButton, *editItemSerialQuantity, *editItemSerialLabel, *editItemTable;
} EditItem;

//int i_handler = 2; // --> Controls which mode to select on the button selection entry modes.
//int itemNotExist = 0;
//gchar *editItemEntry = "\0"; // --> Must fill this with null data to prevent segmentation faults in updateData.


enum {
	ID,
	BARCODE,
	DESCRIPTION,
	EXTRAINFO,
	MANUFACTURER,
	CATEGORY,
	REPLACE,
	WEIGHT,
	LENGTH,
	WIDTH,
	HEIGHT,
	DISCONTINUED,
	COST,
	COSTAVG,
	COSTAVGTTL,
	PROFIT,
	MARGIN,
	PRICE,
	DEALER,
	STOCK,
	LOWSTOCKLVL,
	TOTAL_SOLD,
	TOTAL_SOLD_AMOUNT,
	TOTAL_SOLD_AMOUNTF,
	LAST_SOLD,
	COSTF,
	COSTAVGF,
	COSTAVGTTLF,
	PROFITF,
	MARGINF,
	PRICEF,
	WEIGHTF,
	LENGTHI,
	WIDTHI,
	HEIGHTI,
	STOCKF,
	LOWSTOCKLVLF,
	YEARTOTAL,
	YEARTOTALINT,
	YEAREST,
	YEARESTINT,
	YEARTOTALAMOUNT,
	YEARTOTALAMOUNTF,
	YEARESTAMOUNT,
	YEARESTAMOUNTF,
	INVENTORY_COLUMNS
};


//const char *importNames[] = {PART_COLUMN,DESC_COLUMN,MANU_COLUMN,REPLACE_COLUMN,WEIGHT_COLUMN,DISC_COLUMN,COST_COLUMN,COSTAVG_COLUMN,COSTAVGTTL_COLUMN,PROFIT_COLUMN,PRICE_COLUMN,DEALER_COLUMN,STOCK_COLUMN,ORDER_COLUMN,TOTALSOLD_COLUMN,TOTALSOLDAMOUNT_COLUMN,LASTSOLD_COLUMN};
//const int column_counter = 16;

#define PART_COLUMN "Part #"
#define DESC_COLUMN "Description"
#define EXTRA_COLUMN "Extra Info"
#define MANU_COLUMN "Manufacturer"
#define CAT_COLUMN "Category"
#define REPLACE_COLUMN "Replace"
#define WEIGHT_COLUMN "Weight (kg)"
#define LENGTH_COLUMN "Length (mm)"
#define HEIGHT_COLUMN "Height (mm)"
#define WIDTH_COLUMN "Width (mm)"
#define DISC_COLUMN "Discontinued"
#define COST_COLUMN "Cost"
#define COSTAVG_COLUMN "Avg Cost"
#define COSTAVGTTL_COLUMN "Avg Cost TTL"
#define PROFIT_COLUMN "Profit"
#define PRICE_COLUMN "Price"
#define MARGIN_COLUMN "Margin"
#define DEALER_COLUMN "Dlr Disc."
#define STOCK_COLUMN "Stock"
#define ORDER_COLUMN "Order LVL"
#define TOTALSOLD_COLUMN "TTL Sold"
#define TOTALSOLDAMOUNT_COLUMN "TTL Sold $"
#define LASTSOLD_COLUMN "Last Sold"

void initalizeInventory(GtkBuilder *, gchar *, gchar *, GtkWidget *);
static void initalizeWidgets(GtkBuilder *);
static void openCategories(GtkWidget *, GtkWidget *);
static void prepareExport(GtkWidget *, intrackInventory *);
static void prepareImport(GtkWidget *, intrackInventory *);
static void prepareAddItem(GtkWidget *, intrackInventory *);
static void ColumnHideUnhide(GtkTreeViewColumn *, intrackInventory *);
static void ColumnButtonHideUnhide(GtkWidget *, intrackInventory *);

static void calculateTreeTotals(GtkTreeStore *, intrackInventory *);

static int cellClickedBarcode(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedDescription(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedExtraInfo(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedCategory(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedManufacturer(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);

static int cellClickedReplace(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedCost(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedWeight(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedPrice(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedLength(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedWidth(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedHeight(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);

//static int cellClickedDealer(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedDealer(GtkCellRendererToggle *, gchar *, intrackInventory *);

static int cellClickedStock(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedLowStockLVL(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedDiscontinued(GtkCellRendererToggle *, gchar *, intrackInventory *);

static int cellClickedSerialReq(GtkCellRendererToggle *, gchar *, intrackInventory *);
//static void cellClickedSerialReq(GtkCellRendererToggle *, gchar *, intrackInventory *);
static void changeSerialReq(GtkTreeRowReference *, intrackInventory *);

int databaseEditItemByID(gchar *, gchar *, gchar *, gchar *, gchar *);
static int databaseRemoveItem(gchar *, gchar *, gchar *);
static void prepareItemRemoval(GtkWidget *, intrackInventory *);
static void beginItemRemoval(GtkTreeRowReference *, intrackInventory *);

static int databaseEditItem(gchar *, gchar *, gchar *, gchar *, gchar *);
static int databaseQuery(gchar *, gchar *);
static int getCountSold(gchar *);

static int pullInventory(intrackInventory *, GtkTreeStore *, gchar *, gchar *, gchar *, gchar *, gchar *);

static void deleteItemWindow(GtkWidget *, intrackInventory *);
static void setupInventoryTree(intrackInventory *);
static void getInventory(GtkWidget *, intrackInventory *);
static void getInventoryNumbers(GtkWidget *widget, intrackInventory *inventory);
static void initalizeSearchComboBox(GtkWidget *);

static void hideGtkWidget(GtkWidget *, gpointer);
static void destroyGTKWidget(GtkWidget *, gpointer);

static gboolean treeKeyPress(GtkWidget *, GdkEventKey *, intrackInventory *);
static gboolean deleteItem_key_press(GtkWidget *, GdkEventKey *, intrackInventory *);
static gboolean treeButtonPress(GtkWidget *, GdkEventButton *, intrackInventory *);
static gboolean selectionTimer(gpointer);
static gboolean keyPressSelection(gpointer);
static gboolean urlOpen(gpointer);

static void cell_data_func(GtkTreeViewColumn *, GtkCellRenderer *, GtkTreeModel *, GtkTreeIter *, gpointer);
static void initalizeHideUnhideCounters(intrackInventory *);

static void startEditCategory(GtkTreeRowReference *, intrackInventory *);

static void keyPressGetRow(GtkTreeRowReference *, intrackInventory *);
static void prepareViewWindow(GtkWidget *, intrackInventory *);
static void prepareUriLink(GtkWidget *, intrackInventory *);
static void prepareEditWindow(GtkWidget *, intrackInventory *);
static void prepareHideColumns(GtkWidget *, intrackInventory *);

int databaseTaxFeesItemsEdit(gchar *, gchar *, int, gchar *, gchar *, gchar *);
static int databaseTaxFeesItemsEdit2(gchar *, gchar *, gchar *, gchar *);
static int databaseTaxFeesItemsRemove(gchar *, gchar *, gchar *);

static float getAverageCost(gchar *);
static int checkIfExist(gchar *barCode);
