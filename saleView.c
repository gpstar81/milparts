//      saleView.c
//      Copyright 2011 Michael Rajotte <michael@michaelrajotte.com>

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
#include "saleView.h"

/*
TODO:
*** MEMORY LEAKS: GtkTreePath *path; <- search for that term and make sure to put a free_path(path). All the key press selections in intrack need to be fixed. ***
GREEN = PAID
RED = PAST DUE UNPAID
WHITE = NOTHING
	 
LAYOUT:
-------------------------------------------------
|				  TOOL BAR						| -> View Invoice, Mark As Paid, Mark As Unpaid, Edit Terms, Close
-------------------------------------------------	 
---------------------------------
|								|
|		 Account Info			| -> Not going to have this part. Not needed. Redundant information.
|								|
---------------------------------
|								|
|	 	tree_list_store			|
|		  of invoices			| -> Have a "paid" toggle button for each listed invoice which changes it's status and updates the account balance. Spin button to edit terms length.
|	 colour coded on status     |
|								|
---------------------------------	 
*/

void loadSaleView(GtkWidget *mainWindow, gchar *invoiceNumber, gchar *accountNumber) {
	accountsView	*viewAccount;
	
	viewAccount = g_slice_new(accountsView);
	viewAccount->invoiceNumber = g_strdup(invoiceNumber);
	viewAccount->accountNumber = g_strdup(accountNumber);
	
	GtkBuilder *builder;
    builder = gtk_builder_new();
    
    // Load UI from file.
	gtk_builder_add_from_file(builder, SALES_VIEW_FILE, NULL);
	
	viewAccount->window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	g_signal_connect(viewAccount->window, "destroy", G_CALLBACK(freeWindow), viewAccount);
	
	gchar	*windowTitle = g_strconcat("Parts Order # ", invoiceNumber, NULL);
	/* Setup the top level window */
    gtk_window_set_title(GTK_WINDOW(viewAccount->window), windowTitle);
    g_free(windowTitle);

	gtk_window_set_modal(GTK_WINDOW(viewAccount->window), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(viewAccount->window), TRUE);
	gtk_window_set_deletable(GTK_WINDOW(viewAccount->window), FALSE);
	gtk_window_set_position(GTK_WINDOW(viewAccount->window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(viewAccount->window), 0);
	gtk_window_set_transient_for(GTK_WINDOW(viewAccount->window), GTK_WINDOW(mainWindow));
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(viewAccount->window), TRUE);

	/* Toolbar buttons */
	GtkWidget	*closeButton;
	closeButton = GTK_WIDGET(gtk_builder_get_object(builder, "closeButton"));
	g_signal_connect(G_OBJECT(closeButton), "clicked", G_CALLBACK(destroyWindow), viewAccount->window);

	/* Setup the parts tree */
	viewAccount->viewContainer = GTK_WIDGET(gtk_builder_get_object(builder, "viewScroll"));
	viewAccount->viewTree = gtk_tree_view_new();
	setupViewTree(viewAccount);
	gtk_container_add(GTK_CONTAINER(viewAccount->viewContainer), viewAccount->viewTree);
	
	/* Setup the invoice tree */
	viewAccount->viewContainer2 = GTK_WIDGET(gtk_builder_get_object(builder, "viewScroll2"));
	viewAccount->viewTree2 = gtk_tree_view_new();
	setupViewTreeInfo(viewAccount);
	gtk_container_add(GTK_CONTAINER(viewAccount->viewContainer2), viewAccount->viewTree2);
	
	/* Setup keypress signals on the tree */
	//g_signal_connect(viewAccount->viewTree, "button-press-event", G_CALLBACK(treeButtonPress), viewAccount);
	//g_signal_connect(viewAccount->viewTree, "key-press-event", G_CALLBACK(treeKeyPress), viewAccount);
	
	getInvoices(NULL, viewAccount);
	
	gtk_widget_show_all(viewAccount->window);
	g_object_unref(G_OBJECT(builder));	
}

/* Part one of pulling account invoice data */
static void getInvoices(GtkWidget *widget, accountsView *viewAccount) {
	GtkListStore *store;
	GtkListStore *store2;

	/* Create a new tree model with three columns, as string, gint and guint. */
	store = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	store2 = gtk_list_store_new(COLUMNS_VIEW, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	pullInvoices(store, viewAccount);
	pullInvoicesDetail(store2, viewAccount);

	/* Add the tree model to the tree view and unreference it so that the model will be destroyed along with the tree view. */
	gtk_tree_view_set_model(GTK_TREE_VIEW(viewAccount->viewTree), GTK_TREE_MODEL (store));
	gtk_tree_view_set_model(GTK_TREE_VIEW(viewAccount->viewTree2), GTK_TREE_MODEL (store2));
	
	g_object_unref(store);
	g_object_unref(store2);
}

static int pullInvoicesDetail(GtkListStore *store, accountsView *viewAccount) {
	GtkTreeIter 	iter;
	gchar *query_string;
	int query_state;
	int i;
	int num_fields;		
	
	/* Open MYSQL connection to the database */
	if(connectToServer() == 1) {
		return 1;
	}
	
    // Select the database.
    query_string = g_strdup("millerSales");	
	query_state = mysql_select_db(connection, query_string);	
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, viewAccount->window);
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}		
	
	query_string = g_strconcat("SELECT id, email, name, company, address, city, province, country, code, phone, fax, notes, orderPO FROM `", SALES_TABLE, "` WHERE saleNo='", viewAccount->invoiceNumber, "' AND userID='", viewAccount->accountNumber,"'", NULL);

	query_state = mysql_query(connection, query_string);
	g_free(query_string);
	
	// Failed to connect and query database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, viewAccount->window);
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}
	
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);	
	
	while((row = mysql_fetch_row(result))) {
		gchar *email, *name, *company, *address, *city, *province;
		gchar *country, *code, *phone, *fax, *notes, *orderPO;

		for(i = 0; i < num_fields; i++) {
			if(i == 1)
				email = g_strdup(row[i]);
			
			if(i == 2)
				name = g_strdup(row[i]);
				
			if(i == 3)
				company = g_strdup(row[i]);
				
			if(i == 4)
				address = g_strdup(row[i]);
				
			if(i == 5)
				city = g_strdup(row[i]);

			if(i == 6)
				province = g_strdup(row[i]);
				
			if(i == 7)
				country = g_strdup(row[i]);
				
			if(i == 8)
				code = g_strdup(row[i]);
				
			if(i == 9)
				phone = g_strdup(row[i]);
				
			if(i == 10)
				fax = g_strdup(row[i]);
				
			if(i == 11)
				notes = g_strdup(row[i]);
				
			if(i == 12)
				orderPO = g_strdup(row[i]);
		}
		
		gtk_list_store_append (store, &iter);
		//gtk_list_store_set(store, &iter, SALE_QTY, qty, SALE_PARTNUMBER, partNo, SALE_PARTDESC, partDescription, SALE_PRICEEACH, price, SALE_PRICE, priceTotal, -1);
		gtk_list_store_set(store, &iter, S_EMAIL, email, S_NAME, name, S_COMPANY, company, S_ADDRESS, address, S_CITY, city, S_PROV, province, S_COUNTRY, country, S_CODE, code, S_PHONE, phone, S_FAX, fax, S_NOTES, notes, S_PO, orderPO, -1);

		g_free(email), g_free(name), g_free(company), g_free(address), g_free(city), g_free(province);
		g_free(country), g_free(code), g_free(phone), g_free(fax), g_free(notes), g_free(orderPO);
	}
	
	mysql_free_result(result);
	mysql_close(connection);	
	
	return 0;
}

static int pullInvoices(GtkListStore *store, accountsView *viewAccount) {
	GtkTreeIter 	iter;
	gchar *query_string;
	int query_state;
	int i;
	int num_fields;		
	
	/* Open MYSQL connection to the database */
	if(connectToServer() == 1) {
		return 1;
	}
	
    // Select the database.
    //query_string = g_strdup("millerAccounts");	
    query_string = g_strdup("millerSales");	
	query_state = mysql_select_db(connection, query_string);	
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, viewAccount->window);
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}		
	
	query_string = g_strconcat("SELECT id, qty, partNo, price FROM `",ITEM_SALES_TABLE,"` WHERE saleNo='", viewAccount->invoiceNumber, "' AND userID='",viewAccount->accountNumber,"'", NULL);

	query_state = mysql_query(connection, query_string);
	g_free(query_string);
	
	// Failed to connect and query database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, viewAccount->window);
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}
	
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);	
	
	while((row = mysql_fetch_row(result))) {
		gchar *qty, *partNo, *price;

		for(i = 0; i < num_fields; i++) {
			if(i == 1)
				qty = g_strdup(row[i]);
			
			if(i == 2)
				partNo = g_strdup(row[i]);
				
			if(i == 3)
				price = g_strdup(row[i]);
		}
		
		gchar *priceTotal = g_strdup_printf("%.2f", atof(price) * atoi(qty));
		gchar *partDescription = getPartDescription(partNo, viewAccount->window);
		
		gtk_list_store_append (store, &iter);
		gtk_list_store_set(store, &iter, SALE_QTY, qty, SALE_PARTNUMBER, partNo, SALE_PARTDESC, partDescription, SALE_PRICEEACH, price, SALE_PRICE, priceTotal, -1);
			
		g_free(qty), g_free(partNo), g_free(price), g_free(partDescription), g_free(priceTotal);
	}
	
	mysql_free_result(result);
	mysql_close(connection);	
	
	return 0;
}

/* Adds up the total sales for a individual account */
static gchar *getPartDescription(gchar *partNo, GtkWidget *mainWindow) {
	gchar *query_string;
	int query_state;
	int i;
	//int num_fields;		
	
	MYSQL *accountConnection, accountMysql;
	MYSQL_RES *accountResult;
	MYSQL_ROW accountRow;

	mysql_init(&accountMysql);

	accountConnection = mysql_real_connect(&accountMysql,mysqlServer,mysqlUsername,mysqlPassword,mysqlDatabase,0,NULL,0);

	// taxes uses a separate connection, if connection fails, check the network connection
	if(accountConnection == NULL) {
		printf(mysql_error(accountConnection), "%d\n");
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(accountConnection));

		printMessage(errorMessage, mainWindow);
		mysql_close(accountConnection);
		//return 1;
	}
	
    // Select the database.
    query_string = g_strdup("millerparts");	
	query_state = mysql_select_db(accountConnection, query_string);
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(accountConnection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(accountConnection));
		printMessage(errorMessage, mainWindow);
		g_free(errorMessage);
		
		mysql_close(accountConnection);
		//return 1;
	}		
	
	query_string = g_strconcat("SELECT description FROM `", INVENTORY_TABLES, "` WHERE partNo ='", partNo, "'", NULL);
	query_state = mysql_query(accountConnection, query_string);
	g_free(query_string);
	
	// Failed to connect and query database.
	if (query_state != 0) {
		printf(mysql_error(accountConnection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(accountConnection));
		printMessage(errorMessage, mainWindow);
		g_free(errorMessage);
		
		mysql_close(accountConnection);
		
		//return 1;
	}	
	
	accountResult = mysql_store_result(accountConnection);
	//num_fields = mysql_num_fields(accountResult);	
	int partHere = 0;
	gchar *var;
	
	while((accountRow = mysql_fetch_row(accountResult))) {
		//partDescription = g_strdup(accountRow[0]);
		var = g_strdup(accountRow[0]);
		partHere++;
	}
	
	mysql_free_result(accountResult);
	mysql_close(accountConnection);
	
	if(partHere > 0)
		return var;
		//return g_strdup(var);
	else
		return g_strdup("");
}

/* Gets the balance of the current account */
static gdouble getAccountBalance(GtkWidget *window, gchar *account) {
	gchar *query_string;
	int query_state;	
	
	if(connectToServer() == 1) {
		return 0;
	}
	
	// Select the database.
	query_string = g_strdup(mysqlDatabase);
	query_state = mysql_select_db(connection, query_string);
	g_free(query_string);
		
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s\n%s", ERROR_DATABASE, mysql_error(connection));
		printMessage(errorMessage, window);
		g_free(errorMessage);		
		
		mysql_close(connection);
		
		return 0;
	}
	
	query_string = g_strconcat("SELECT id, balance FROM `", ACCOUNTS_TABLES, "` WHERE number='", account, "'", NULL);
	query_state = mysql_query(connection, query_string);
	
	g_free(query_string);
	
	/* Failed to get the data */
	if(query_state !=0) {
		printf(mysql_error(connection), "%d\n");

		gchar *errorMessage = g_strdup_printf("%s\n%s", ERROR_UPDATING_ACCOUNT, mysql_error(connection));
		printMessage(errorMessage, window);
		g_free(errorMessage);	
				
		mysql_close(connection);
		return 0;
	}
	
	int num_fields;
	int i;	
	gdouble balance = 0.00;
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);	

	while((row = mysql_fetch_row(result))) {
		for(i = 0; i < num_fields; i++)	
			if(i == 1)
				balance = atof(row[i]);
	}
	
	mysql_free_result(result);
	mysql_close(connection);			
	
	return balance;
}

/* Create the tree view and columns */
static void setupViewTree(accountsView *viewAccount) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Qty", renderer, "text", SALE_QTY, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SALE_QTY); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Part #", renderer, "text", SALE_PARTNUMBER, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SALE_PARTNUMBER); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Description", renderer, "text", SALE_PARTDESC, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SALE_PARTDESC); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree), column);	

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Price Ea.", renderer, "text", SALE_PRICEEACH, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SALE_PRICEEACH); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("SubTotal", renderer, "text", SALE_PRICE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SALE_PRICE); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree), column);	
}

/* Create the tree view and columns */
static void setupViewTreeInfo(accountsView *viewAccount) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Email", renderer, "text", S_EMAIL, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, S_EMAIL); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree2), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", S_NAME, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, S_NAME); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree2), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Company", renderer, "text", S_COMPANY, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, S_COMPANY); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree2), column);	

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Address", renderer, "text", S_ADDRESS, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, S_ADDRESS); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree2), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("City", renderer, "text", S_CITY, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, S_CITY); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree2), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Province", renderer, "text", S_PROV, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, S_PROV); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree2), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Country", renderer, "text", S_COUNTRY, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, S_COUNTRY); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree2), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Postal Code", renderer, "text", S_CODE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, S_CODE); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree2), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Phone", renderer, "text", S_PHONE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, S_PHONE); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree2), column);	

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Fax", renderer, "text", S_FAX, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, S_FAX); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree2), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Notes", renderer, "text", S_NOTES, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, S_NOTES); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree2), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("PO #", renderer, "text", S_PO, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, S_PO); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (viewAccount->viewTree2), column);		
}

static void destroyWindow(GtkWidget *widget, GtkWidget *window) {
	gtk_widget_destroy(GTK_WIDGET(window));
}

/* Memory Management */
static void freeWindow(GtkWidget *widget, accountsView *viewAccount) {
	if(widget)
		gtk_widget_destroy(widget);
	
	freeMemory(viewAccount);
}

/* Free up memory */
static void freeMemory(accountsView *viewAccount) {
	//g_free(selectedAccount);
	g_free(viewAccount->invoiceNumber);
	g_free(viewAccount->accountNumber);
	g_slice_free(accountsView, viewAccount);
}
