//      inventory_cat_structs.h
//      Copyright 2010-2014 Michael Rajotte <michael@michaelrajotte.com>
// 		Creating item categories and adding items to them.

typedef struct _ItemContainer {
	GtkWidget		*mainWindow;
	GtkWidget		*cancelButton, *okButton;
	
	GtkWidget		*inventoryContainer, *inventoryTree;
	GtkWidget		*inventorySearchEntry, *barcodeSearch, *nameSearch, *descriptionSearch, *manufacturerSearch, *categorySearch, *inventorySearchButton;
	GtkWidget		*costSearch, *priceSearch, *stockSearch, *inventoryNumberSearchButton;
	GtkWidget		*searchSpinMin, *searchSpinMax;
	
	GtkWidget		*invenMenu, *invenMenuView;
	
	gchar	*data_query;
	gchar	*selectedItemCode;
	int		counter;
} ItemContainer;

typedef struct _intrackCategories {
	GtkWidget	*catTree, *catViewport;
	GtkWidget	*catInventoryTree, *catInventoryViewport;
	//GtkWidget	*inventoryTree, *inventoryViewport;

	/*
	GtkWidget	*inventorySearchEntry;
	GtkWidget	*barcodeSearch, *nameSearch, *descriptionSearch, *manufacturerSearch, *categorySearch; 
	GtkWidget	*costSearch, *priceSearch, *stockSearch;
	GtkWidget	*searchSpinMin, *searchSpinMax;
	*/
	//GtkWidget	*addCatButton, *deleteCatButton;
	GtkWidget	*addCatButtonBar, *deleteCatButtonBar;
	//GtkWidget	*addCatItems, *deleteCatItems;
	GtkWidget	*addCatItemsBar, *deleteCatItemsBar;
	
	//GtkWidget	*catMenu, *catMenuAdd, *catMenuRemove;
	GtkWidget	*invenMenu, *invenMenuView;
	GtkWidget	*mainWindow; /* The main program widget window */
	
	gchar 	*selectedCat;
	gchar	*selectedItemCode;
	
	ItemContainer *itemContainer;
}intrackCategories;

typedef struct _intrackCatEntry {
	GtkWidget	*inputWindow, *table, *inputEntry, *inputLabel;
	GtkWidget	*sendButton, *cancelButton;
}intrackCatEntry;
