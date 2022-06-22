//      inventory_hidemenu.h
//      Copyright 2010-2014 Michael Rajotte <michael@michaelrajotte.com>
// 		Hide / Unhide columns in the inventory.c gtktree.
/*
typedef struct _IntrackHide {
	GtkWidget	*addButton, *cancelButton;
	
	//GtkWidget 	*part_button, *desc_button, *manu_button, *replace_button, *weight_button, *disc_button, *cost_button, *costavg_button, *costavgttl_button, *profit_button, *price_button, *dealer_button, *stock_button, *order_button, *totalsold_button, *totalsoldamount_button, *lastsold_button, *extrainfo_button;
}IntrackHide;
*/
GtkWidget	*mainWindow;

GtkWidget 	*part_button, *desc_button, *manu_button, *replace_button, *weight_button, *disc_button, *cost_button, *costavg_button, *costavgttl_button, *profit_button, *price_button, *dealer_button, *stock_button, *order_button, *totalsold_button, *totalsoldamount_button, *lastsold_button, *extrainfo_button, *category_button, *length_button, *width_button, *height_button, *margin_button, *ytd_button, *est_button, *ytd_amount_button, *est_amount_button;

void initalizeHideMenu(GtkWidget *, intrackInventory *);

static void destroyWindow(GtkWidget *, GtkWidget *);
//static void freeMemory(GtkWidget *, IntrackHide *);
static void initalizeButtons(intrackInventory *);
static void updateColumns(GtkWidget *, intrackInventory *);

