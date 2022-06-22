//      partSales.h
//      Copyright 2010 Michael Rajotte <michael@michaelrajotte.com>
// 		For inventory control, mass listing, searching and editing via a gtktree.

#include "calendar.h"

typedef struct _intrackInventory {
	GtkWidget	*inventoryTree, *inventoryViewport;
	GtkTreeSelection *selection;
	
	GtkDateEntry	*dateEntryFrom; // -> calendar.h
	GtkDateEntry	*dateEntryTo; // -> calendar.h

	GtkWidget	*returnButton;
	GtkWidget	*inventorySearchEntry;
	GtkWidget	*barcodeSearch, *descriptionSearch, *soldToSearch; 
	GtkWidget	*costSearch, *priceSearch;
	GtkWidget	*searchSpinMin, *searchSpinMax;
	
	//GtkWidget	*invenMenu, *invenMenuView, *invenMenuRemove, *invenMenuEdit;
	//GtkWidget	*invenMenu, *invenMenuRemove;
	
	/* One of the Toolbar buttons. Sensitvity changes on this. Thats why it is in the struct. */
	//GtkWidget	*invenViewItemButton, *removeItemButton, *editItemButton; 
	//GtkWidget	*removeItemButton;
	
	GtkWidget	*numberOfPartsLabel, *partCostsLabel, *partSalesLabel, *partProfitsLabel, *partMarginLabel;
	GtkWidget	*numberOfPartsLabel1, *partCostsLabel1, *partSalesLabel1, *partProfitsLabel1, *partMarginLabel1;
	GtkWidget	*numberOfPartsLabelPrev, *partCostsLabelPrev, *partSalesLabelPrev, *partProfitsLabelPrev, *partMarginLabelPrev;
	GtkWidget	*numberOfPartsLabelDiff, *partCostsLabelDiff, *partSalesLabelDiff, *partProfitsLabelDiff, *partMarginLabelDiff;
	GtkWidget	*numberOfPartsLabelPerc, *partCostsLabelPerc, *partSalesLabelPerc, *partProfitsLabelPerc, *partMarginLabelPerc;

	gchar 		*mysqlDatabase;
	gchar		*inventoryTable;
	gchar		*selectedItemCode;
	gchar		*exportQueryString;
	
	int		numberOfParts;
	int		numberOfPartsPrev;
	
	float	partCosts, partSales, partProfits, partMargin;
	float	partCostsPrev, partSalesPrev, partProfitsPrev, partMarginPrev;
	
	GtkWidget	*mainWindow; /* The main program widget window */
}intrackInventory;

typedef struct _removeSale {
	gchar *whereData, *partNo, *oldStockData, *costData, *oldAvgCostData, *oldPrice;
}removeSale;

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
	COST,
	PROFIT,
	MARGIN,
	PRICE,
	SOLDTO,
	LAST_SOLD,
	INVOICENO,
	COSTF,
	PROFITF,
	PRICEF,
	MARGINF,
	INVENTORY_COLUMNS
};

void initalizePartSales(GtkBuilder *, gchar *, gchar *, GtkWidget *);
static void initalizeWidgets(GtkBuilder *);
static void openCategories(GtkWidget *, GtkWidget *);
static void prepareExport(GtkWidget *, intrackInventory *);
static void prepareImport(GtkWidget *, intrackInventory *);
static void prepareAddItem(GtkWidget *, GtkWidget *);
static void prepareSaleAddItem(GtkWidget *, intrackInventory *);
static void prepareCalculate(GtkWidget *, intrackInventory *);
static void beginCalculate(GtkTreeRowReference *, intrackInventory *);
static void cell_data_func(GtkTreeViewColumn *, GtkCellRenderer *, GtkTreeModel *, GtkTreeIter *, gpointer);

static void calculateTreeTotals(GtkListStore *, intrackInventory *);

static int databaseRemoveItem(gchar *, gchar *, gchar *);
static void prepareItemRemoval(GtkWidget *, intrackInventory *);
static void beginItemRemoval(GtkTreeRowReference *, intrackInventory *);
static int getItemData(intrackInventory *, removeSale *, gchar *);

static int cellClickedDescription(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedDateSold(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedPartNo(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);

static int cellClickedSoldTo(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedPrice(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedCost(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);
static int cellClickedInvoice(GtkCellRendererText *, gchar *, gchar *, intrackInventory *);


static int databaseEditItem(gchar *, gchar *, gchar *, gchar *, gchar *);
static int databaseQuery(gchar *, gchar *);

static int pullInventory(intrackInventory *, GtkListStore *, gchar *, gchar *, gchar *, gchar *, gchar *, GDate *, GDate *, gboolean, gboolean);

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

static void keyPressGetRow(GtkTreeRowReference *, intrackInventory *);
static void prepareViewWindow(GtkWidget *, intrackInventory *);
static void prepareEditWindow(GtkWidget *, intrackInventory *);

//int databaseTaxFeesItemsEdit(gchar *, gchar *, int, gchar *, gchar *, gchar *);
static int databaseTaxFeesItemsEdit2(gchar *, gchar *, gchar *, gchar *);
static int databaseTaxFeesItemsRemove(gchar *, gchar *, gchar *);
static int databaseReturnStockItem(gchar *, gchar *, gchar *, gchar *, gchar *);

static float getAverageCost(gchar *);
static int checkIfExist(gchar *barCode);
static int check_date(gchar *);
