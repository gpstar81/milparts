//      inventory_add.c
//      Copyright 2010-2014 Michael Rajotte <michael@michaelrajotte.com>
// 		Manually adding a item to the database.

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
#include "inventory_add.h"

/*
TODO:
*/


void initalizeAdd(GtkWidget *parentWindow, intrackInventory *inventory) {
	GtkBuilder *builder;
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, INVENTORY_ADD_FILE, NULL);

	IntrackAdd *intrackAdd;
	intrackAdd = (IntrackAdd*) g_malloc (sizeof (IntrackAdd));
	
	// Setup the top level window
	intrackAdd->mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "addWindow"));
	gtk_window_set_title (GTK_WINDOW(intrackAdd->mainWindow), "Add Item");
	gtk_window_set_transient_for(GTK_WINDOW(intrackAdd->mainWindow), GTK_WINDOW(parentWindow));
	gtk_window_set_modal(GTK_WINDOW(intrackAdd->mainWindow), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(intrackAdd->mainWindow), FALSE);
	gtk_widget_set_size_request(intrackAdd->mainWindow, 324, 510);
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
	//intrackAdd->imageLabel = GTK_WIDGET(gtk_builder_get_object(builder, "imageLabel"));
	intrackAdd->imageButton = GTK_WIDGET(gtk_builder_get_object(builder, "imageButton"));
	//g_signal_connect(intrackAdd->imageButton, "clicked", G_CALLBACK(imageChooserDialog), intrackAdd);
	
	/* The entry fields and spin buttons */
	intrackAdd->codeEntry = GTK_WIDGET(gtk_builder_get_object(builder, "codeEntry"));
	//intrackAdd->nameEntry = GTK_WIDGET(gtk_builder_get_object(builder, "nameEntry"));
	intrackAdd->descriptionEntry = GTK_WIDGET(gtk_builder_get_object(builder, "descriptionEntry"));
	intrackAdd->extrainfoEntry = GTK_WIDGET(gtk_builder_get_object(builder, "extrainfoEntry"));

	intrackAdd->replaceEntry = GTK_WIDGET(gtk_builder_get_object(builder, "replaceEntry"));
	intrackAdd->qty = GTK_WIDGET(gtk_builder_get_object(builder, "qtySpin"));
	intrackAdd->lowStockLvl = GTK_WIDGET(gtk_builder_get_object(builder, "lowStockLvlSpin"));
	intrackAdd->cost = GTK_WIDGET(gtk_builder_get_object(builder, "costSpin"));
	intrackAdd->price = GTK_WIDGET(gtk_builder_get_object(builder, "priceSpin"));
	intrackAdd->dealer = GTK_WIDGET(gtk_builder_get_object(builder, "dealerSpin"));
	intrackAdd->categoryEntry = GTK_WIDGET(gtk_builder_get_object(builder, "categoryEntry"));
	intrackAdd->manufacturerEntry = GTK_WIDGET(gtk_builder_get_object(builder, "manufacturerEntry"));
	intrackAdd->numSerials = GTK_WIDGET(gtk_builder_get_object(builder, "serialSpin"));

	intrackAdd->length = GTK_WIDGET(gtk_builder_get_object(builder, "lengthspin"));
	intrackAdd->height = GTK_WIDGET(gtk_builder_get_object(builder, "heightspin"));
	intrackAdd->width = GTK_WIDGET(gtk_builder_get_object(builder, "widthspin"));
	intrackAdd->weight = GTK_WIDGET(gtk_builder_get_object(builder, "weightspin"));

	//intrackAdd->duty = GTK_WIDGET(gtk_builder_get_object(builder, "dutySpin"));
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
	gchar *itemCode = g_strconcat(gtk_entry_get_text(GTK_ENTRY(intrackAdd->codeEntry)), NULL);
	
	if(checkIfExist(itemCode) < 1) {
		printMessage("ERROR: Item already exists.", intrackAdd->mainWindow);
	}
	else if(gtk_entry_get_text_length(GTK_ENTRY(intrackAdd->codeEntry)) < 1) {
		printMessage("ERROR: Code length is too short.", intrackAdd->mainWindow);
	}
	else {
		dataState = createData(intrackAdd);
	}

	g_free(itemCode);
	
	if(dataState == 0)
		destroyWindow(NULL, intrackAdd->mainWindow);
}

static int createData(IntrackAdd *intrackAdd) {
	/* Connection Error */
	if(connectToServer() == 1)
		return 1;
	
	gchar *query_string;
	int qty, lowStockLvl, numserials, length, width, height;
	float price, dealer, cost, duty, weight;
	gchar *qtyChar, *lowStockLvlChar, *priceChar, *dealerChar, *costChar;
	gchar *numserialsChar, *lengthChar, *heightChar, *widthChar, *weightChar;
	
	qty = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(intrackAdd->qty));
	lowStockLvl = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(intrackAdd->lowStockLvl));
	price = gtk_spin_button_get_value(GTK_SPIN_BUTTON(intrackAdd->price));
	dealer = gtk_spin_button_get_value(GTK_SPIN_BUTTON(intrackAdd->dealer));
	cost = gtk_spin_button_get_value(GTK_SPIN_BUTTON(intrackAdd->cost));
	numserials = gtk_spin_button_get_value(GTK_SPIN_BUTTON(intrackAdd->numSerials));

	length = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(intrackAdd->length));
	width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(intrackAdd->width));
	height = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(intrackAdd->height));
	weight = gtk_spin_button_get_value(GTK_SPIN_BUTTON(intrackAdd->weight));

	qtyChar = g_strdup_printf("%i", qty);
	lowStockLvlChar = g_strdup_printf("%i", lowStockLvl);
	priceChar = g_strdup_printf("%.2f", price);
	dealerChar = g_strdup_printf("%.2f", dealer);
	costChar = g_strdup_printf("%.2f", cost);
	numserialsChar = g_strdup_printf("%i", numserials);
	lengthChar = g_strdup_printf("%i", length);
	widthChar = g_strdup_printf("%i", width);
	heightChar = g_strdup_printf("%i", height);
	weightChar = g_strdup_printf("%.3f", weight);
		
	//query_string = g_strconcat("INSERT INTO ", mysqlTables, " (barcode, name, description, manufacturer, category, stock, lowStockLvl, price, cost, costAverage, duty, serialRequired, numberOfSerial, countryOrigin) VALUES ('", gtk_entry_get_text(GTK_ENTRY(intrackAdd->codeEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->nameEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->descriptionEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->replaceEntry)), "', '", gtk_label_get_text(GTK_LABEL(intrackAdd->catLabel)), "', '", qtyChar, "', '", lowStockLvlChar, "', '", priceChar, "', '", costChar, "', '", costChar, "', '", dutyChar, "', '", serialReqChar, "', '", numSerialChar, "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->countryEntry)), "')", NULL);
	
	query_string = g_strconcat("INSERT INTO ", mysqlTables, " (partNo, description, replacePart, cost, costAvg, price, dealer, stock, lowStockLvl, category, manufacturer, numserials, shipwidth, shiplength, shipheight, weight, extraInfo) VALUES ('", gtk_entry_get_text(GTK_ENTRY(intrackAdd->codeEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->descriptionEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->replaceEntry)), "', '", costChar, "', '", costChar, "', '", priceChar, "', '", dealerChar, "', '", qtyChar, "', '", lowStockLvlChar, "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->categoryEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->manufacturerEntry)), "', '", numserialsChar, "', '", widthChar, "', '", lengthChar, "', '", heightChar, "', '", weightChar, "', '", gtk_entry_get_text(GTK_ENTRY(intrackAdd->extrainfoEntry)), "')", NULL);
	
	g_free(qtyChar), g_free(lowStockLvlChar), g_free(priceChar), g_free(dealerChar), g_free(costChar);
	g_free(numserialsChar), g_free(lengthChar), g_free(heightChar), g_free(widthChar), g_free(weightChar);
	
	//g_print("%s\n", query_string);

	int query_state;
	query_state = mysql_query(connection, query_string);
	g_free(query_string);

	if(query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_TABLES, intrackAdd->mainWindow);

		mysql_close(connection);
		return 1;
	}
	else {
		// Begin file processing.
		//gchar *fileName = g_strconcat(gtk_label_get_text(GTK_LABEL(intrackAdd->imageLabel)), NULL); 
		gchar *fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(intrackAdd->imageButton));
		
		// If file upload exists
		if(fileName != NULL && strlen(fileName) > 0) {
			// Binary file buffer stuff. Limit image size to 1 megabyte.
			int len, size;
			char data[1000*1024];
			// 1000 * 1024
			char chunk[2*1000*1024+1];
			char query[1024*5000];
			
			FILE *fp;
			fp = fopen(fileName, "rb");
			size = fread(data, 1, 1024*1000, fp);
			mysql_real_escape_string(connection, chunk, data, size);

			char *stat = g_strconcat("UPDATE ", mysqlTables, " SET imageData = '%s' WHERE partNo='", gtk_entry_get_text(GTK_ENTRY(intrackAdd->codeEntry)), "'", NULL);

			len = snprintf(query, sizeof(stat)+sizeof(chunk) , stat, chunk);

			mysql_real_query(connection, query, len);
			
			fclose(fp);
			g_free(stat);
		}

		g_free(fileName);		
	}
		
	mysql_close(connection);

	GError     *error;
	gchar *uri;
	
	uri = g_strconcat("http://barcode.millercanada.com/createlabel.php?label=standard&code=", gtk_entry_get_text(GTK_ENTRY(intrackAdd->codeEntry)), "&description=", gtk_entry_get_text(GTK_ENTRY(intrackAdd->descriptionEntry)), NULL);
	
	gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &error);
	g_free(uri);
	
	
	
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
