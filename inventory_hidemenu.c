//      inventory_hidemenu.c
//      Copyright 2010-2014 Michael Rajotte <michael@michaelrajotte.com>
// 		Hide / Unhide columns in the inventory.c gtktree.

#include <mysql.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <time.h>
#include <gdk/gdkkeysyms.h>
#include "settings.h"
#include "messages.h"
#include "inventory.h"
#include "inventory_hidemenu.h"

/*
TODO:
*/

/*
int int_part_button = 0;
int int_desc_button = 0;
int int_extra_button = 0;
int int_manu_button = 0;
int int_replace_button = 0;
int int_weight_button = 0;
int int_disc_button = 0;
int int_cost_button = 0;
int int_costavgttl_button = 0;
int int_profit_button = 0;
int int_price_button = 0;
int int_dealer_button = 0;
int int_order_button = 0;
int int_stock_button = 0;
int int_costavg_button = 0;
int int_totalsold_button = 0;
int int_totalsoldamount_button = 0;
int int_lastsold_button = 0;
*/

void initalizeHideMenu(GtkWidget *parentWindow, intrackInventory *inventory) {
	GtkBuilder *builder;
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, INVENTORY_HIDE_FILE, NULL);

	//IntrackHide *intrackHide;
	//intrackHide = (IntrackHide*) g_malloc (sizeof (IntrackHide));
	
	// Setup the top level window
	mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "hideWindow"));
	gtk_window_set_title (GTK_WINDOW(mainWindow), "Hide / Unhide Columns");
	gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));
	gtk_window_set_modal(GTK_WINDOW(mainWindow), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(mainWindow), FALSE);
	gtk_widget_set_size_request(mainWindow, 324, 510);
	gtk_window_set_deletable(GTK_WINDOW(mainWindow), FALSE);
	gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(mainWindow), 0);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(mainWindow), TRUE);
	//g_signal_connect(mainWindow, "destroy", G_CALLBACK(freeMemory), intrackHide);
	
	GtkWidget	*applyButton;
	applyButton = GTK_WIDGET(gtk_builder_get_object(builder, "addButton"));
	gtk_button_set_label(GTK_BUTTON(applyButton), "Apply");
	g_signal_connect(applyButton, "clicked", G_CALLBACK(updateColumns), inventory);

	GtkWidget	*cancelButton;
	cancelButton = GTK_WIDGET(gtk_builder_get_object(builder, "cancelButton"));
	g_signal_connect(cancelButton, "clicked", G_CALLBACK(destroyWindow), mainWindow);
	
	// Setup hide / unhide buttons.
	part_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton0"));
	gtk_button_set_label(GTK_BUTTON(part_button), "Part #");
	
	desc_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton1"));
	gtk_button_set_label(GTK_BUTTON(desc_button), "Description");
	
	extrainfo_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton2"));
	gtk_button_set_label(GTK_BUTTON(extrainfo_button), "Extra Info");
	
	manu_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton3"));
	gtk_button_set_label(GTK_BUTTON(manu_button), "Manufacturer");
	
	replace_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton4"));
	gtk_button_set_label(GTK_BUTTON(replace_button), "Replace");
	
	weight_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton5"));
	gtk_button_set_label(GTK_BUTTON(weight_button), "Weight");
	
	disc_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton6"));
	gtk_button_set_label(GTK_BUTTON(disc_button), "Discontinued");
	
	cost_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton7"));
	gtk_button_set_label(GTK_BUTTON(cost_button), "Cost");
	
	costavg_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton8"));
	gtk_button_set_label(GTK_BUTTON(costavg_button), "Cost Average");
	
	costavgttl_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton9"));
	gtk_button_set_label(GTK_BUTTON(costavgttl_button), "Cost Average Total");
	
	profit_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton10"));
	gtk_button_set_label(GTK_BUTTON(profit_button), "Profit");
	
	price_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton11"));
	gtk_button_set_label(GTK_BUTTON(price_button), "Price");
	
	dealer_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton12"));
	gtk_button_set_label(GTK_BUTTON(dealer_button), "Dealer");
	
	stock_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton13"));
	gtk_button_set_label(GTK_BUTTON(stock_button), "Stock");
	
	order_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton14"));
	gtk_button_set_label(GTK_BUTTON(order_button), "Order Level");
	
	totalsold_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton15"));
	gtk_button_set_label(GTK_BUTTON(totalsold_button), "Total Sold");
	
	totalsoldamount_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton16"));
	gtk_button_set_label(GTK_BUTTON(totalsoldamount_button), "Total Sold $");
	
	lastsold_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton17"));
	gtk_button_set_label(GTK_BUTTON(lastsold_button), "Last Sold");
	//---
	category_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton18"));
	gtk_button_set_label(GTK_BUTTON(category_button), "Category");
	
	length_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton19"));
	gtk_button_set_label(GTK_BUTTON(length_button), "Length (mm)");
	
	width_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton20"));
	gtk_button_set_label(GTK_BUTTON(width_button), "Width (mm)");	

	height_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton21"));
	gtk_button_set_label(GTK_BUTTON(height_button), "Height (mm)");
	
	margin_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton22"));
	gtk_button_set_label(GTK_BUTTON(margin_button), "Margin");
	
	ytd_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton23"));
	gtk_button_set_label(GTK_BUTTON(ytd_button), "Ytd");	

	est_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton24"));
	gtk_button_set_label(GTK_BUTTON(est_button), "Est");
	
	ytd_amount_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton25"));
	gtk_button_set_label(GTK_BUTTON(ytd_amount_button), "Ytd $");
	
	est_amount_button = GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton26"));
	gtk_button_set_label(GTK_BUTTON(est_amount_button), "Est $");	
			
	initalizeButtons(inventory);
	
    g_object_unref(G_OBJECT(builder));
    
	gtk_widget_show_all(mainWindow);
}

static void destroyWindow(GtkWidget *widget, GtkWidget *mainWindow) {
	gtk_widget_destroy(GTK_WIDGET(mainWindow));
}

/*
static void freeMemory(GtkWidget *widget, IntrackHide *intrackHide) {
	if(widget) {
		destroyWindow(NULL, mainWindow);
		g_free(intrackHide);
	}
}
*/

static void updateColumns(GtkWidget *widget, intrackInventory *inventory) {
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(part_button)))
		inventory->int_part_button = 1;
	else
		inventory->int_part_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(desc_button)))
		inventory->int_desc_button = 1;
	else
		inventory->int_desc_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(extrainfo_button)))
		inventory->int_extra_button = 1;
	else
		inventory->int_extra_button = 0;		
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(manu_button)))
		inventory->int_manu_button = 1;
	else
		inventory->int_manu_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(replace_button)))
		inventory->int_replace_button = 1;
	else
		inventory->int_replace_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(weight_button)))
		inventory->int_weight_button = 1;
	else
		inventory->int_weight_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(disc_button)))
		inventory->int_disc_button = 1;
	else
		inventory->int_disc_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cost_button)))
		inventory->int_cost_button = 1;
	else
		inventory->int_cost_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(costavg_button)))
		inventory->int_costavg_button = 1;
	else
		inventory->int_costavg_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(costavgttl_button)))
		inventory->int_costavgttl_button = 1;
	else
		inventory->int_costavgttl_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(profit_button)))
		inventory->int_profit_button = 1;
	else
		inventory->int_profit_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(price_button)))
		inventory->int_price_button = 1;
	else
		inventory->int_price_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dealer_button)))
		inventory->int_dealer_button = 1;
	else
		inventory->int_dealer_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(stock_button)))
		inventory->int_stock_button = 1;
	else
		inventory->int_stock_button = 0;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(order_button)))
		inventory->int_order_button = 1;
	else
		inventory->int_order_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(totalsold_button)))
		inventory->int_totalsold_button = 1;
	else
		inventory->int_totalsold_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(totalsoldamount_button)))
		inventory->int_totalsoldamount_button = 1;
	else
		inventory->int_totalsoldamount_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lastsold_button)))
		inventory->int_lastsold_button = 1;
	else
		inventory->int_lastsold_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(category_button)))
		inventory->int_category_button = 1;
	else
		inventory->int_category_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(length_button)))
		inventory->int_length_button = 1;
	else
		inventory->int_length_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(width_button)))
		inventory->int_width_button = 1;
	else
		inventory->int_width_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(height_button)))
		inventory->int_height_button = 1;
	else
		inventory->int_height_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(margin_button)))
		inventory->int_margin_button = 1;
	else
		inventory->int_margin_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ytd_button)))
		inventory->int_ytd_button = 1;
	else
		inventory->int_ytd_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(est_button)))
		inventory->int_est_button = 1;
	else
		inventory->int_est_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ytd_amount_button)))
		inventory->int_ytd_amount_button = 1;
	else
		inventory->int_ytd_amount_button = 0;
		
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(est_amount_button)))
		inventory->int_est_amount_button = 1;
	else
		inventory->int_est_amount_button = 0;		
		
	gtk_widget_destroy(GTK_WIDGET(mainWindow));
}

static void initalizeButtons(intrackInventory *inventory) {
	if(inventory->int_part_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(part_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(part_button), FALSE);
		
	if(inventory->int_desc_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(desc_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(desc_button), FALSE);
		
	if(inventory->int_extra_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(extrainfo_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(extrainfo_button), FALSE);
		
	if(inventory->int_manu_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(manu_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(manu_button), FALSE);
		
	if(inventory->int_replace_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(replace_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(replace_button), FALSE);
		
	if(inventory->int_weight_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(weight_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(weight_button), FALSE);
		
	if(inventory->int_disc_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(disc_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(disc_button), FALSE);
		
	if(inventory->int_cost_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cost_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cost_button), FALSE);
		
	if(inventory->int_costavgttl_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(costavgttl_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(costavgttl_button), FALSE);
		
	if(inventory->int_profit_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(profit_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(profit_button), FALSE);
		
	if(inventory->int_price_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(price_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(price_button), FALSE);
		
	if(inventory->int_dealer_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dealer_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dealer_button), FALSE);
		
	if(inventory->int_order_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(order_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(order_button), FALSE);
		
	if(inventory->int_stock_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stock_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stock_button), FALSE);
		
	if(inventory->int_costavg_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(costavg_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(costavg_button), FALSE);
			
	if(inventory->int_totalsold_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(totalsold_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(totalsold_button), FALSE);
			
	if(inventory->int_lastsold_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lastsold_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lastsold_button), FALSE);
			
	if(inventory->int_totalsoldamount_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(totalsoldamount_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(totalsoldamount_button), FALSE);
		
	if(inventory->int_category_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(category_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(category_button), FALSE);		

	if(inventory->int_length_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(length_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(length_button), FALSE);
		
	if(inventory->int_width_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(width_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(width_button), FALSE);
		
	if(inventory->int_height_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(height_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(height_button), FALSE);
		
	if(inventory->int_margin_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(margin_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(margin_button), FALSE);
		
	if(inventory->int_ytd_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ytd_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ytd_button), FALSE);
		
	if(inventory->int_est_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(est_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(est_button), FALSE);
		
	if(inventory->int_ytd_amount_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ytd_amount_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ytd_amount_button), FALSE);
		
	if(inventory->int_est_amount_button == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(est_amount_button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(est_amount_button), FALSE);			
}
