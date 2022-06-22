//      systems_add.c
//      Copyright 2010-2011 Michael Rajotte <michael@michaelrajotte.com>
// 		Manually adding a system item to the database.

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
#include "systems_add.h"

/*
TODO:
*/


void initalizeSystemAdd(GtkWidget *parentWindow) {
	GtkBuilder *builder;
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, SYSTEMS_ADDITEM_FILE, NULL);
	
	IntrackAdd *intrackAdd;
	intrackAdd = (IntrackAdd*) g_malloc (sizeof (IntrackAdd));
	
	/* Setup the top level window */
	intrackAdd->mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "addWindow"));
	gtk_window_set_title (GTK_WINDOW(intrackAdd->mainWindow), "Systems Add Item");
	gtk_window_set_transient_for(GTK_WINDOW(intrackAdd->mainWindow), GTK_WINDOW(parentWindow));
	gtk_window_set_modal(GTK_WINDOW(intrackAdd->mainWindow), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(intrackAdd->mainWindow), FALSE);
	gtk_widget_set_size_request(intrackAdd->mainWindow, 324, 315);
	gtk_window_set_deletable(GTK_WINDOW(intrackAdd->mainWindow), FALSE);
	gtk_window_set_position(GTK_WINDOW(intrackAdd->mainWindow), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(intrackAdd->mainWindow), 0);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(intrackAdd->mainWindow), TRUE);
	g_signal_connect(intrackAdd->mainWindow, "destroy", G_CALLBACK(freeMemory), intrackAdd);
	
	/* Apply Button */
	GtkWidget	*addButton;
	addButton = GTK_WIDGET(gtk_builder_get_object(builder, "addButton"));
	g_signal_connect(addButton, "clicked", G_CALLBACK(sendData), intrackAdd);
	
	/* Cancel Button */
	GtkWidget	*cancelButton;
	cancelButton = GTK_WIDGET(gtk_builder_get_object(builder, "cancelButton"));
	g_signal_connect(cancelButton, "clicked", G_CALLBACK(destroyWindow), intrackAdd->mainWindow);
	
	/* Category Button */
	/*
	GtkWidget	*catButton;
	catButton = GTK_WIDGET(gtk_builder_get_object(builder, "catButton"));
	g_signal_connect(catButton, "clicked", G_CALLBACK(prepareLoadCat), intrackAdd);	
	*/
	
	/* Category Label */
	//intrackAdd->catLabel = GTK_WIDGET(gtk_builder_get_object(builder, "catLabel"));

	/* Image Button */
	/*
	intrackAdd->imageLabel = GTK_WIDGET(gtk_builder_get_object(builder, "imageLabel"));
	intrackAdd->imageButton = GTK_WIDGET(gtk_builder_get_object(builder, "imageButton"));
	g_signal_connect(intrackAdd->imageButton, "clicked", G_CALLBACK(imageChooserDialog), intrackAdd);
	*/
	
	/* The entry fields and spin buttons */
	intrackAdd->catEntry = GTK_WIDGET(gtk_builder_get_object(builder, "catEntry"));
	intrackAdd->serial1Entry = GTK_WIDGET(gtk_builder_get_object(builder, "serial1Entry"));
	intrackAdd->serial2Entry = GTK_WIDGET(gtk_builder_get_object(builder, "serial2Entry"));
	intrackAdd->serial3Entry = GTK_WIDGET(gtk_builder_get_object(builder, "serial3Entry"));
	intrackAdd->dateInEntry = GTK_WIDGET(gtk_builder_get_object(builder, "dateInEntry"));
	intrackAdd->dateOutEntry = GTK_WIDGET(gtk_builder_get_object(builder, "dateOutEntry"));
	intrackAdd->soldToEntry = GTK_WIDGET(gtk_builder_get_object(builder, "soldToEntry"));
	intrackAdd->invoiceEntry = GTK_WIDGET(gtk_builder_get_object(builder, "invoiceEntry"));
	//intrackAdd->qty = GTK_WIDGET(gtk_builder_get_object(builder, "qtySpin"));
	//intrackAdd->lowStockLvl = GTK_WIDGET(gtk_builder_get_object(builder, "lowStockLvlSpin"));
	//intrackAdd->cost = GTK_WIDGET(gtk_builder_get_object(builder, "costSpin"));
	//intrackAdd->price = GTK_WIDGET(gtk_builder_get_object(builder, "priceSpin"));
	//intrackAdd->duty = GTK_WIDGET(gtk_builder_get_object(builder, "dutySpin"));
	//intrackAdd->numSerials = GTK_WIDGET(gtk_builder_get_object(builder, "serialSpin"));
	//intrackAdd->countryEntry = GTK_WIDGET(gtk_builder_get_object(builder, "countryEntry"));

    g_object_unref(G_OBJECT(builder));
    
	gtk_widget_show_all(intrackAdd->mainWindow);
}

static void destroyWindow(GtkWidget *widget, GtkWidget *mainWindow) {
	gtk_widget_destroy(GTK_WIDGET(mainWindow));
}

static void freeMemory(GtkWidget *widget, IntrackAdd *intrackAdd) {
	if(widget) {
		destroyWindow(NULL, intrackAdd->mainWindow);
		g_free(intrackAdd);
	}
}

// -> checkout_manual_cat.c
static void prepareLoadCat(GtkWidget *widget, IntrackAdd *intrackAdd) {
	//categoryCheckoutPopup(intrackAdd->mainWindow, intrackAdd->catLabel); // -> checkout_manual_cat.c
}

static void imageChooserDialog(GtkWidget *widget, IntrackAdd *intrackAdd) {
	
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new ("Open File",
				      GTK_WINDOW(intrackAdd->mainWindow),
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

    gtk_filter_add(dialog, "JPEG image (*.jpg, *.jpeg, *.jpe)", "*.jpg"); 
    gtk_filter_add(dialog, "PNG image (*.png)", "*.png");

	if(gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		//gtk_label_set_text(GTK_LABEL(intrackAdd->imageLabel), gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)));
	}
	
	gtk_widget_destroy (dialog);
}

static void sendData(GtkWidget *widget, IntrackAdd *intrackAdd) {
	int dataState;
	
	if(gtk_entry_get_text_length(GTK_ENTRY(intrackAdd->catEntry)) < 1) {
		printMessage("ERROR: Code length is too short.", intrackAdd->mainWindow);
	}
	else {
		// Item Exists, so proceed.
		dataState = createData(intrackAdd);
	}

	if(dataState == 0)
		destroyWindow(NULL, intrackAdd->mainWindow);
}

static int createData(IntrackAdd *intrackAdd) {
	MYSQL *partsConnection, partsMysql;
	MYSQL_RES *partsResult;
	MYSQL_ROW partsRow;		
	
	int query_state;
	gchar *query_string;

	/* Open connection to the database */
	mysql_init(&partsMysql);
	partsConnection = mysql_real_connect(&partsMysql,mysqlServer,mysqlUsername,mysqlPassword,MYSQL_DEFAULT_DATABASE, 0, 0, 0);
	
	if(partsConnection == NULL) {
		printf(mysql_error(partsConnection), "%d\n");

		printMessage("CONNECTION FAILED", NULL);
		mysql_close(partsConnection);
		return 1;
	}	
	
    // Select the database.
    query_string = g_strdup(SYSTEMS_DATABASE);	
	query_state = mysql_select_db(partsConnection, query_string);
	g_free(query_string);	
	
	// Insert the system items.
	query_string = g_strconcat("INSERT INTO `", SYSTEMS_TABLES, "` (catNum, serial1, serial2, serial3, dateIn, dateOut, soldTo, invoiceNum) VALUES ('", gtk_entry_get_text(GTK_ENTRY(intrackAdd->catEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->serial1Entry)), "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->serial2Entry)), "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->serial3Entry)), "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->dateInEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->dateOutEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->soldToEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->invoiceEntry)), "')", NULL);

	query_state = mysql_query(partsConnection, query_string);
	g_free(query_string);
		
	mysql_close(partsConnection);

	return 0;
}

/* Updates a item in the inventory database. */
static int databaseQuery(gchar *database, gchar *query) {
	
	int query_state;	
	
	if(connectToServer() == 1) {
		return 1;
	}
	
	// Select the database.
	query_state = mysql_select_db(connection, database);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, NULL);
		g_free(errorMessage);		
		
		mysql_close(connection);
		
		return 1;
	}
	
	query_state=mysql_query(connection, query);
	
	/* Failed to update the data */
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, NULL);
		g_free(errorMessage);	
				
		mysql_close(connection);

		return 1;
	}
	
	mysql_close(connection);			
	return 0;	
}

static int checkIfExist(gchar *barCode) {

	/* Connection Error */
	if(connectToServer() == 1)
		return 1;
	
	gchar *query_string;

	query_string = g_strconcat("SELECT partNo, stock FROM `", mysqlTables, "` WHERE partNo = '", barCode, "'", NULL);
	
	int query_state;
	query_state = mysql_query(connection, query_string);
	
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
	}
	g_free(query_string);

	//If the connection is successful and the query returns a result then the next step is to display those results:
	result = mysql_store_result(connection);

	// If the item is not in the database, then stop and return back.
	if (!mysql_fetch_row(result)) {
		mysql_free_result(result); // Free up some memory.
		mysql_close(connection);
	
		// Set global variable item does not exist.
		//itemNotExist = 0;

		gchar *messageText = g_strconcat(barCode, " Item Not Found", NULL);
		//printMessage(messageText, NULL);
		g_free(messageText);

		return 1;
	}
	
	//itemNotExist = 1;

	mysql_free_result(result); // Free up some memory.
	mysql_close(connection);	

	return 0;
}
