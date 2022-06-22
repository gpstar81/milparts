//      accounts_view.c
//      Copyright 2011 Michael Rajotte <michael@michaelrajotte.com>
// 		For listing account and highlighting invoices via colour coding the cells for there status.

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
#include "accounts_structs.h"
#include "accounts_view.h"

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

void loadAccountsView(intrackAccounts *accounts) {
	accounts->viewAccount = g_slice_new(accountsView);
	accounts->viewAccount->dateEntryFrom = g_slice_new(GtkDateEntry);
	accounts->viewAccount->dateEntryTo = g_slice_new(GtkDateEntry);
	accounts->viewAccount->path = g_strdup(NULL);
	accounts->viewAccount->invoiceNumber = g_strdup(NULL);
	
	GtkBuilder *builder;
    builder = gtk_builder_new();
    
    // Load UI from file.
	gtk_builder_add_from_file(builder, ACCOUNTS_VIEW_FILE, NULL);
	
	accounts->viewAccount->window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	g_signal_connect(accounts->viewAccount->window, "destroy", G_CALLBACK(freeWindow), accounts);
	
	/* Setup the top level window */
    gtk_window_set_title(GTK_WINDOW(accounts->viewAccount->window), "Miller Parts View Account");
	gtk_window_set_modal(GTK_WINDOW(accounts->viewAccount->window), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(accounts->viewAccount->window), TRUE);
	gtk_window_set_deletable(GTK_WINDOW(accounts->viewAccount->window), FALSE);
	gtk_window_set_position(GTK_WINDOW(accounts->viewAccount->window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(accounts->viewAccount->window), 0);
	gtk_window_set_transient_for(GTK_WINDOW(accounts->viewAccount->window), GTK_WINDOW(accounts->mainWindow));
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(accounts->viewAccount->window), TRUE);
	
	/* Toolbar buttons */
	GtkWidget	*closeButton;
	closeButton = GTK_WIDGET(gtk_builder_get_object(builder, "closeButton"));
	g_signal_connect(G_OBJECT(closeButton), "clicked", G_CALLBACK(destroyWindow), accounts->viewAccount->window);

	/* Paid, Unpaid and View Invoice toolbar buttons */
	accounts->viewAccount->invoiceButton = GTK_WIDGET(gtk_builder_get_object(builder, "viewButton"));
	gtk_widget_set_sensitive(accounts->viewAccount->invoiceButton, FALSE);
	g_signal_connect(G_OBJECT(accounts->viewAccount->invoiceButton), "clicked", G_CALLBACK(prepareLoadInvoice), accounts);

	/* Setup the accounts tree */
	accounts->viewAccount->viewContainer = GTK_WIDGET(gtk_builder_get_object(builder, "viewScroll"));
	accounts->viewAccount->viewTree = gtk_tree_view_new();
	setupViewTree(accounts);
	gtk_container_add(GTK_CONTAINER(accounts->viewAccount->viewContainer), accounts->viewAccount->viewTree);
	
	/* Setup keypress signals on the tree */
	g_signal_connect(accounts->viewAccount->viewTree, "button-press-event", G_CALLBACK(treeButtonPress), accounts);
	g_signal_connect(accounts->viewAccount->viewTree, "key-press-event", G_CALLBACK(treeKeyPress), accounts);
	
	/* Setup the search entry & button */
	accounts->viewAccount->searchEntry = GTK_WIDGET(gtk_builder_get_object(builder, "searchEntry"));
	g_signal_connect(G_OBJECT(accounts->viewAccount->searchEntry), "activate", G_CALLBACK(getInvoices), accounts);
	
	GtkWidget *searchButton = GTK_WIDGET(gtk_builder_get_object(builder, "searchButton"));
	g_signal_connect(G_OBJECT(searchButton), "clicked", G_CALLBACK(getInvoices), accounts);
	
	// Calendar Mode Combo Box
	//accounts->viewAccount->calendarContainer = GTK_WIDGET(gtk_builder_get_object(builder, "calendarContainer"));
	//accounts->viewAccount->calendarComboContainer = GTK_CONTAINER(gtk_builder_get_object(builder, "calendarComboContainer"));
	
	/* Now set the sensitivity of the calendar widgets */	
	//calendarSensitivity(NULL, accounts);
	// EOF Calendar Mode Combo Box

	/* Setup the drop down calendars */
	setupCalendar(builder, accounts->viewAccount->dateEntryFrom, accounts->viewAccount->dateEntryTo);
	g_signal_connect(accounts->viewAccount->window, "show", G_CALLBACK(getInvoices), accounts);

	gtk_widget_show_all(accounts->viewAccount->window);
	g_object_unref(G_OBJECT(builder));	
}

/* Load up the invoice -> saleView.c */
static void prepareLoadInvoice(GtkWidget *widget, intrackAccounts *accounts) {
	loadSaleView(accounts->viewAccount->window, accounts->viewAccount->invoiceNumber, accounts->selectedAccount); /* -> saleView.c */
}


/* Part one of pulling account invoice data */
static void getInvoices(GtkWidget *widget, intrackAccounts *accounts) {
	GtkListStore *store;

	/* Create a new tree model with three columns, as string, gint and guint. */
	store = gtk_list_store_new(COLUMNS_VIEW, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	/* Load the tree data with the search options if the search button was pressed. */
	if(widget != NULL) {
		gchar *search = g_strdup(gtk_entry_get_text(GTK_ENTRY(accounts->viewAccount->searchEntry)));
		GDate *dateFrom = g_date_new();
		GDate *dateTo = g_date_new();
		
		processDate(dateFrom, accounts->viewAccount->dateEntryFrom->entry); // functions.h
		processDate(dateTo, accounts->viewAccount->dateEntryTo->entry); // functions.h
		
		pullInvoices(store, accounts, search, dateFrom, dateTo);

		g_free(search);
		g_date_free(dateFrom), g_date_free(dateTo);
	}

	/* Add the tree model to the tree view and unreference it so that the model will be destroyed along with the tree view. */
	gtk_tree_view_set_model(GTK_TREE_VIEW(accounts->viewAccount->viewTree), GTK_TREE_MODEL (store));

	//calculateTreeTotals(store, intrackCalendar);
	/* Reset the sensitivity on the paid, unpaid and invoice view buttons */
	gtk_widget_set_sensitive(accounts->viewAccount->invoiceButton, FALSE);
	
	/* Reset and clear out the search entry */
	gtk_entry_set_text(GTK_ENTRY(accounts->viewAccount->searchEntry), "");

	g_object_unref(store);
}

static int pullInvoices(GtkListStore *store, intrackAccounts *accounts, gchar *search, GDate *dateFrom, GDate *dateTo) {
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
		printMessage(errorMessage, accounts->viewAccount->window);
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}		
	
	/* Search for the user entered query */
	if(search != NULL)
		query_string = g_strconcat("SELECT id, saleNo, orderDate, orderTotal FROM `", SALES_TABLE, "` WHERE userID='", accounts->selectedAccount,"' AND saleNo LIKE '%", search, "%'", NULL);
	else
		query_string = g_strconcat("SELECT id, saleNo, orderDate, orderTotal FROM `", SALES_TABLE, "` WHERE userID='", accounts->selectedAccount,"' LIMIT 0, 1000", NULL);

	query_state = mysql_query(connection, query_string);
	g_free(query_string);
	
	// Failed to connect and query database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, accounts->viewAccount->window);
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}	
	
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);	
	
	while((row = mysql_fetch_row(result))) {
		gchar *saleNo, *orderDate, *totalSales;

		for(i = 0; i < num_fields; i++) {
			if(i == 1)
				saleNo = g_strdup(row[i]);
			
			if(i == 2)
				orderDate = g_strdup(row[i]);
				
			if(i == 3)			
				totalSales = g_strdup(row[i]);				
		}
		
		/* Processing The Dates */
		GDate *dateInvoiced, *dateToday;
		int yearScan, monthScan, dayScan;
		gchar	dueDateBuffer[256];
		
		dateInvoiced = g_date_new();
		dateToday = g_date_new();
		
		g_date_set_time_t(dateToday, time(NULL)); // Get today's date.
				
		sscanf(orderDate, "%d-%d-%d", &yearScan, &monthScan, &dayScan);
		gchar *dateStrTemp = g_strdup_printf("%d-%d-%d", yearScan, monthScan, dayScan);

		g_date_set_parse(dateInvoiced, dateStrTemp);
		//g_date_add_days(dateTemp, atoi(terms));
		//g_date_strftime(dueDateBuffer, 256, "%Y-%m-%d", dateTemp);
		
		gboolean validInvoiceDate = FALSE;
		if(g_date_compare(dateInvoiced, dateFrom) >=0) {
			if(g_date_compare(dateInvoiced, dateTo) <= 0)
				validInvoiceDate = TRUE;
			else
				validInvoiceDate = FALSE;
		}
		else
			validInvoiceDate = FALSE;
		/* EOF Processing The Dates */
			
		/* Store the results into the tree now */
		if(validInvoiceDate) {
			/* Calculate the total sales for the account */
			/*gchar *totalSales;
			gdouble totalSalesDouble = 0.00;
			totalSalesDouble = getTotalSales(accounts->selectedAccount, saleNo, accounts->viewAccount->window);
			totalSales = g_strdup_printf("%.2f", totalSalesDouble);			
			*/	
			gtk_list_store_append (store, &iter);
			gtk_list_store_set(store, &iter, SALE_NO, saleNo, SALE_TOTAL, totalSales, SALE_DATE, orderDate, -1);
			
			
		}
		
		g_free(saleNo), g_free(orderDate);
		g_date_free(dateToday), g_date_free(dateInvoiced), g_free(dateStrTemp);
		g_free(totalSales);
	}
	
	mysql_free_result(result);
	mysql_close(connection);	
	
	return 0;
}

/* Adds up the total sales for a individual account */
static gdouble getTotalSales(gchar *accountNo, gchar *saleNo, GtkWidget *mainWindow) {
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
		return 1;
	}
	
    // Select the database.
    query_string = g_strdup("millerAccounts");	
	query_state = mysql_select_db(accountConnection, query_string);
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(accountConnection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(accountConnection));
		printMessage(errorMessage, mainWindow);
		g_free(errorMessage);
		
		mysql_close(accountConnection);
		return 1;
	}		
	
	query_string = g_strconcat("SELECT qty, price FROM `", accountNo, "` WHERE saleNo ='", saleNo, "'", NULL);
	query_state = mysql_query(accountConnection, query_string);
	g_free(query_string);
	
	// Failed to connect and query database.
	if (query_state != 0) {
		printf(mysql_error(accountConnection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(accountConnection));
		printMessage(errorMessage, mainWindow);
		g_free(errorMessage);
		
		mysql_close(accountConnection);
		
		return 1;
	}	
	
	accountResult = mysql_store_result(accountConnection);
	//num_fields = mysql_num_fields(accountResult);	
	
	gdouble totalSales = 0.00;
	while((accountRow = mysql_fetch_row(accountResult))) {
		//totalSales = totalSales + atof(accountRow[0]);
		totalSales = totalSales + (atoi(accountRow[0]) * atof(accountRow[1]));
	}
	
	mysql_free_result(accountResult);
	mysql_close(accountConnection);	
	
	return totalSales;
}

/* Updates a account and invoice in the database. */
static int accountsUpdate(GtkWidget *window, gchar *database, gchar *query) {
	
	gchar *query_string;
	int query_state;	
	
	if(connectToServer() == 1) {
		return 1;
	}
	
	// Select the database.
	query_string = g_strdup(database);
	query_state = mysql_select_db(connection, query_string);
	g_free(query_string);
		
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s\n%s", ERROR_DATABASE, mysql_error(connection));
		printMessage(errorMessage, window);
		g_free(errorMessage);		
		
		mysql_close(connection);
		
		return 1;
	}
	
	//query_string = g_strconcat("UPDATE `", ACCOUNTS_TABLES, "`", " SET ", column, "='", newData, "' WHERE number='", accountNumber, "'", NULL);
	query_state=mysql_query(connection, query);
		
	/* Failed to update the data */
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");

		gchar *errorMessage = g_strdup_printf("%s\n%s", ERROR_UPDATING_ACCOUNT, mysql_error(connection));
		printMessage(errorMessage, window);
		g_free(errorMessage);	
				
		mysql_close(connection);
		return 1;
	}
	
	mysql_close(connection);			
	
	return 0;	
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
static void setupViewTree(intrackAccounts *accounts) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Sale #", renderer, "text", SALE_NO, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SALE_NO); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->viewAccount->viewTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Total", renderer, "text", SALE_TOTAL, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SALE_TOTAL); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->viewAccount->viewTree), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Sale Date", renderer, "text", SALE_DATE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SALE_DATE); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->viewAccount->viewTree), column);
}

/* Setups a calendar. Make Note of the gtkbuilder as it uses certain predetermined widget names in the builder file that you must ahear to. */
static void setupCalendar(GtkBuilder *builder, GtkDateEntry *dateEntryFrom, GtkDateEntry *dateEntryTo) {
	/* ------------------------------------------------------------------------------------------------------------------------------ */
	/* Creates the FROM calendar widget and entry box */
	dateEntryFrom->date = g_date_new();

	/* today's date */
	g_date_set_time_t(dateEntryFrom->date, time(NULL));
	g_date_set_dmy(&dateEntryFrom->mindate, 1, 1, 1900);
	g_date_set_dmy(&dateEntryFrom->maxdate, 31, 12, 2200);
	g_date_subtract_days(dateEntryFrom->date, 365);

	// Entry and button for date from
	gchar	dateBuffer[256];
	g_date_strftime(dateBuffer, 256, "%Y-%m-%d", dateEntryFrom->date);	

	// Entry and button for date from
	dateEntryFrom->entry = GTK_WIDGET(gtk_builder_get_object(builder, "dateEntryFrom"));
	dateEntryFrom->arrow = GTK_WIDGET(gtk_builder_get_object(builder, "fromToggle"));

	g_signal_connect(GTK_OBJECT(dateEntryFrom->arrow), "toggled", G_CALLBACK(gtk_dateentry_arrow_press), dateEntryFrom);
	g_signal_connect(GTK_OBJECT(dateEntryFrom->entry), "activate", G_CALLBACK(gtk_dateentry_entry_new), dateEntryFrom);
	g_signal_connect(GTK_OBJECT(dateEntryFrom->entry), "focus-out-event", G_CALLBACK(gtk_dateentry_focus), dateEntryFrom);

	/* our popup window */
	dateEntryFrom->popwin = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_widget_set_events(dateEntryFrom->popwin, gtk_widget_get_events(dateEntryFrom->popwin) | GDK_KEY_PRESS_MASK);

	dateEntryFrom->frame = gtk_frame_new (NULL);
	gtk_container_add(GTK_CONTAINER(dateEntryFrom->popwin), dateEntryFrom->frame);
	gtk_frame_set_shadow_type (GTK_FRAME(dateEntryFrom->frame), GTK_SHADOW_OUT);
	gtk_widget_show(dateEntryFrom->frame);

	dateEntryFrom->calendar = gtk_calendar_new ();
	gtk_container_add(GTK_CONTAINER(dateEntryFrom->frame), dateEntryFrom->calendar);
	gtk_widget_show(dateEntryFrom->calendar);

	g_signal_connect(GTK_OBJECT(dateEntryFrom->popwin), "key_press_event", G_CALLBACK(key_press_popup), dateEntryFrom);
	g_signal_connect(GTK_OBJECT(dateEntryFrom->popwin), "button_press_event", G_CALLBACK(gtk_dateentry_button_press), dateEntryFrom);

	g_signal_connect(GTK_OBJECT(dateEntryFrom->calendar), "prev-year", G_CALLBACK(gtk_dateentry_calendar_year), dateEntryFrom);
	g_signal_connect(GTK_OBJECT(dateEntryFrom->calendar), "next-year", G_CALLBACK(gtk_dateentry_calendar_year), dateEntryFrom);
	g_signal_connect(GTK_OBJECT(dateEntryFrom->calendar), "prev-month", G_CALLBACK(gtk_dateentry_calendar_year), dateEntryFrom);
	g_signal_connect(GTK_OBJECT(dateEntryFrom->calendar), "next-month", G_CALLBACK(gtk_dateentry_calendar_year), dateEntryFrom);

	g_signal_connect(GTK_OBJECT(dateEntryFrom->calendar), "day-selected", G_CALLBACK(gtk_dateentry_calendar_getfrom), dateEntryFrom);
	g_signal_connect(GTK_OBJECT(dateEntryFrom->calendar), "day-selected-double-click", G_CALLBACK(gtk_dateentry_calendar_select), dateEntryFrom);

	gtk_dateentry_calendar_getfrom(NULL, dateEntryFrom);
	gtk_entry_set_text(GTK_ENTRY(dateEntryFrom->entry), dateBuffer);
	/* ------------------------------------------------------------------------------------------------------------------------------ */

	/* ------------------------------------------------------------------------------------------------------------------------------ */
	/* Creates the TO calendar widget and entry box */
	dateEntryTo->date = g_date_new();
	
	/* today's date */
	g_date_set_time_t(dateEntryTo->date, time(NULL));
	g_date_set_dmy(&dateEntryTo->mindate, 1, 1, 1900);
	g_date_set_dmy(&dateEntryTo->maxdate, 31, 12, 2200);

	// Entry and button for date to
	dateEntryTo->entry = GTK_WIDGET(gtk_builder_get_object(builder, "dateEntryTo"));
	dateEntryTo->arrow = GTK_WIDGET(gtk_builder_get_object(builder, "toToggle"));

	g_signal_connect(GTK_OBJECT(dateEntryTo->arrow), "toggled", G_CALLBACK(gtk_dateentry_arrow_press), dateEntryTo);
	g_signal_connect(GTK_OBJECT(dateEntryTo->entry), "activate", G_CALLBACK(gtk_dateentry_entry_new), dateEntryTo);
	g_signal_connect(GTK_OBJECT(dateEntryTo->entry), "focus-out-event", G_CALLBACK(gtk_dateentry_focus), dateEntryTo);
	g_signal_connect(GTK_OBJECT(dateEntryTo->entry), "key_press_event", G_CALLBACK(gtk_dateentry_entry_key), dateEntryTo);

	/* our popup window */
	dateEntryTo->popwin = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_widget_set_events(dateEntryTo->popwin, gtk_widget_get_events(dateEntryTo->popwin) | GDK_KEY_PRESS_MASK);

	dateEntryTo->frame = gtk_frame_new (NULL);
	gtk_container_add(GTK_CONTAINER(dateEntryTo->popwin), dateEntryTo->frame);
	gtk_frame_set_shadow_type (GTK_FRAME(dateEntryTo->frame), GTK_SHADOW_OUT);
	gtk_widget_show(dateEntryTo->frame);

	dateEntryTo->calendar = gtk_calendar_new ();
	gtk_container_add(GTK_CONTAINER(dateEntryTo->frame), dateEntryTo->calendar);
	gtk_widget_show(dateEntryTo->calendar);

	g_signal_connect(GTK_OBJECT(dateEntryTo->popwin), "key_press_event", G_CALLBACK(key_press_popup), dateEntryTo);
	g_signal_connect(GTK_OBJECT(dateEntryTo->popwin), "button_press_event", G_CALLBACK(gtk_dateentry_button_press),dateEntryTo);

	g_signal_connect(GTK_OBJECT(dateEntryTo->calendar), "prev-year", G_CALLBACK(gtk_dateentry_calendar_year), dateEntryTo);
	g_signal_connect(GTK_OBJECT(dateEntryTo->calendar), "next-year", G_CALLBACK(gtk_dateentry_calendar_year), dateEntryTo);
	g_signal_connect(GTK_OBJECT(dateEntryTo->calendar), "prev-month", G_CALLBACK(gtk_dateentry_calendar_year), dateEntryTo);
	g_signal_connect(GTK_OBJECT(dateEntryTo->calendar), "next-month", G_CALLBACK(gtk_dateentry_calendar_year), dateEntryTo);

	g_signal_connect(GTK_OBJECT(dateEntryTo->calendar), "day-selected", G_CALLBACK(gtk_dateentry_calendar_getfrom), dateEntryTo);
	g_signal_connect(GTK_OBJECT(dateEntryTo->calendar), "day-selected-double-click", G_CALLBACK(gtk_dateentry_calendar_select), dateEntryTo);

	gtk_dateentry_calendar_getfrom(NULL, dateEntryTo);
	/* ------------------------------------------------------------------------------------------------------------------------------ */	
}

/* Mouse click event on the tree */
static gboolean treeButtonPress(GtkWidget *widget, GdkEventButton *ev, intrackAccounts *accounts) {
    GtkTreePath *path;
    
	/* if there's no path where the click occurred... */
    if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(accounts->viewAccount->viewTree), ev->x, ev->y, &path, NULL, NULL, NULL)) {
        return FALSE;
	}

    switch(ev->button)
    {
        case 1: /* 1 = left click */
            break;

        case 3: /* 3 = right click */
            break;
    }
	
	/* Convert the path to a string and keep track of it */
    g_free(accounts->viewAccount->path);
    accounts->viewAccount->path = gtk_tree_path_to_string(path);
    
	/* Keep track of the selected item */
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;  
    gchar 	*invoiceNumber;
    
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->viewAccount->viewTree));
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, SALE_NO, &invoiceNumber, -1);    

	g_free(accounts->viewAccount->invoiceNumber);
	accounts->viewAccount->invoiceNumber = g_strdup(invoiceNumber);
	g_free(invoiceNumber);
	
    /* free our path */
    gtk_tree_path_free(path);
    
	gtk_widget_set_sensitive(accounts->viewAccount->invoiceButton, TRUE);    

    return FALSE;	
}

/* Keyboard key press event on the inventory tree */
static gboolean treeKeyPress(GtkWidget *widget, GdkEventKey *ev, intrackAccounts *accounts) {
    switch(ev->keyval)
    {       
        case GDK_Up:
				/* Add a 1 millisecond delay before pulling the selection from the tree to get accurate selection. */
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;

        case GDK_KP_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;

        case GDK_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;

        case GDK_KP_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;  
            
        case GDK_KP_Home:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;              
            
        case GDK_KP_End:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;     
            
        case GDK_Page_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;  

        case GDK_Page_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;  
            
        case GDK_KP_Page_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;  
            
        case GDK_KP_Page_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;      
            
        case GDK_Home:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;  
            
        case GDK_End:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;
    }
    return FALSE;	
}

static gboolean keyPressSelection(gpointer data) {
	intrackAccounts *accounts = (intrackAccounts *)data;

	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GList *rows, *ptr, *references = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(accounts->viewAccount->viewTree));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->viewAccount->viewTree));
	rows = gtk_tree_selection_get_selected_rows(selection, &model);
	
	ptr = rows;
	// Create tree row references to all of the selected rows
	while(ptr != NULL) {
		ref = gtk_tree_row_reference_new(model, (GtkTreePath*) ptr->data);
		references = g_list_prepend(references, gtk_tree_row_reference_copy(ref));
		gtk_tree_row_reference_free(ref);
		ptr = ptr->next;
	}

	g_list_foreach(references, (GFunc) keyPressGetRow, accounts);

	// Free the tree paths, tree row references and lists
	g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(references);
	g_list_free(rows);
	
	gtk_widget_set_sensitive(accounts->viewAccount->invoiceButton, TRUE);   		

	return FALSE;
}

/* Get the selected row from a key press (part #2) */
static void keyPressGetRow(GtkTreeRowReference *ref, intrackAccounts *accounts) {
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->viewAccount->viewTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

	/* Keep track of the path */
	g_free(accounts->viewAccount->path);
	accounts->viewAccount->path=gtk_tree_path_to_string(path);
	
	/* Keep track of the selected item */
    gchar 	*invoiceNumber;
    gtk_tree_model_get(model, &iter, SALE_NO, &invoiceNumber, -1);    

	g_free(accounts->viewAccount->invoiceNumber);
	accounts->viewAccount->invoiceNumber = g_strdup(invoiceNumber);
	g_free(invoiceNumber); 	

    gtk_tree_path_free(path);
}

static void destroyWindow(GtkWidget *widget, GtkWidget *window) {
	gtk_widget_destroy(GTK_WIDGET(window));
}

/* Memory Management */
static void freeWindow(GtkWidget *widget, intrackAccounts *accounts) {
	if(widget)
		gtk_widget_destroy(widget);
	
	freeMemory(accounts);
}

/* Free up memory */
static void freeMemory(intrackAccounts *accounts) {
	//g_free(accounts->selectedAccount);
	g_free(accounts->viewAccount->path);
	g_free(accounts->viewAccount->invoiceNumber);
	g_slice_free(GtkDateEntry, accounts->viewAccount->dateEntryFrom);
	g_slice_free(GtkDateEntry, accounts->viewAccount->dateEntryTo);
	g_slice_free(accountsView, accounts->viewAccount);
}
