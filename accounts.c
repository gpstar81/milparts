//      accounts.c
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
#include <locale.h>
#include "settings.h"
#include "messages.h"
#include "accounts_structs.h"
#include "accounts.h"

/*
TODO:
(1). Build support to pay partial balances. Ie: sends a cheque and you record the cheque number, the amount and which invoice it is going to, if it is paying partial, etc. A payment history.

YELLOW = BALANCE IS OVER THE LIMIT OR HAS INVOICES PAST DUE
RED = ACCOUNT ON HOLD
WHITE = NOTHING

LAYOUT:
Fee Rates:
-------------------------------------------------
|					TOOL BAR					| -> Create Account, View Account, Delete Account, Close
-------------------------------------------------
		______________
Search [______________]

-------------------------------------------------
|												|
|	name	contact			address		etc		| -> Shows all the account info as listed below.
|	name	contact			address		etc		|
|	name	contact			address		etc		|
|												|
-------------------------------------------------

Database Layout:

Database Name			Table Names			Structure
intrack					accounts			id, number, name, contact, address, city, provstate, code, country, phone, fax, email, onHold, balance (scans thru and adds up all due invoices, can show extra credit as well), accountLimit (max they can have due on all overdue invoices at once)
intrackAccounts			number				id, invoice, invoiceDate, terms, paid, amountDue
*/

/* Loads up the fees configuration */
void loadAccounts(GtkBuilder *builder, GtkWidget *mainWindow) {
	intrackAccounts *accounts;
	accounts = (intrackAccounts*) g_malloc (sizeof (intrackAccounts));
	
	accounts->mainWindow = mainWindow;
	accounts->selectedAccount = g_strdup(NULL);

    /* Exit program button from file / quit. */
    GtkWidget	*exitAccount;
    exitAccount = GTK_WIDGET(gtk_builder_get_object(builder, "accountsCloseButton"));
   	g_signal_connect(G_OBJECT(exitAccount), "activate", G_CALLBACK(hideGtkWidget), mainWindow);	

	/* Toolbar buttons */
	/* View button */
	accounts->viewButton = GTK_WIDGET(gtk_builder_get_object(builder, "viewAccountButton"));
	g_signal_connect(G_OBJECT(accounts->viewButton), "clicked", G_CALLBACK(prepareViewAccount), accounts);
   	gtk_widget_set_sensitive(accounts->viewButton, FALSE);
	
	/* Close the window */
    GtkWidget	*accountCloseButton;
    accountCloseButton = GTK_WIDGET(gtk_builder_get_object(builder, "closeButton"));
   	g_signal_connect(G_OBJECT(accountCloseButton), "clicked", G_CALLBACK(hideGtkWidget), mainWindow);
   	
   	/* Total Sales Label */
   	accounts->totalSalesLabel = GTK_WIDGET(gtk_builder_get_object(builder, "totalSales"));
   	accounts->totalAccountsLabel = GTK_WIDGET(gtk_builder_get_object(builder, "totalAccounts"));
   	
	/* Setup the accounts tree */
	accounts->accountContainer = GTK_WIDGET(gtk_builder_get_object(builder, "accountScroll"));
	accounts->accountTree = gtk_tree_view_new();
	setupAccountTree(accounts);
	gtk_container_add(GTK_CONTAINER(accounts->accountContainer), accounts->accountTree);

	/* Setup the account tree to be multi selection */
	accounts->selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (accounts->accountTree));
	gtk_tree_selection_set_mode(accounts->selection, GTK_SELECTION_MULTIPLE);	

	/* Setup the search entry & button */
	accounts->searchEntry = GTK_WIDGET(gtk_builder_get_object(builder, "searchEntry"));
	g_signal_connect(G_OBJECT(accounts->searchEntry), "activate", G_CALLBACK(getAccounts), accounts);
	
	GtkWidget *searchButton = GTK_WIDGET(gtk_builder_get_object(builder, "searchButton"));
	g_signal_connect(G_OBJECT(searchButton), "clicked", G_CALLBACK(getAccounts), accounts);

	/* Setup keypress signals on the tree */
	g_signal_connect(accounts->accountTree, "button-press-event", G_CALLBACK(treeButtonPress), accounts);
	g_signal_connect(accounts->accountTree, "key-press-event", G_CALLBACK(treeKeyPress), accounts);

	/* Load the inventory tree with all data from the database */
	//getAccounts(NULL, accounts);
	g_signal_connect(accounts->mainWindow, "show", G_CALLBACK(getAccounts), accounts);
}

static void closeWindow(GtkWidget *widget, GtkWidget *window) {
	gtk_widget_destroy(window);
}

static void freeWindow(GtkWidget *widget, intrackAccounts *accounts) {
	if(widget)
		gtk_widget_destroy(widget);
	
	freeMemory(accounts);
}

/* Free up memory after a new account has been created. */
static void freeMemory(intrackAccounts *accounts) {
	//g_free(accounts->selectedAccount);
	g_slice_free(accountsCreate, accounts->newAccount);
}

/* A widget window to ask for confirmation upon deletion */
static void deleteAccountsWindow(GtkWidget *widget, intrackAccounts *accounts) {
	GtkWidget *dialog;
	gint widgetDialog;
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(accounts->mainWindow), 
			GTK_DIALOG_DESTROY_WITH_PARENT, 
			GTK_MESSAGE_WARNING, 
			GTK_BUTTONS_YES_NO, 
			"Delete selected accounts ?");
			
	gtk_window_set_title(GTK_WINDOW(dialog), "Delete Accounts?");
	widgetDialog = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	switch (widgetDialog) {
		case GTK_RESPONSE_YES:
			prepareAccountRemoval(NULL, accounts);
			break;
		case GTK_RESPONSE_NO:
			break;
	}
}

/* Loads the view account widget. accounts_view.c */
static void prepareViewAccount(GtkWidget *widget, intrackAccounts *accounts) {
	loadAccountsView(accounts); /* accounts_view.c */
}

/* Prepares the remove process of accounts from the database */
static void prepareAccountRemoval(GtkWidget *widget, intrackAccounts *accounts) {

	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GList *rows, *ptr, *references = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(accounts->accountTree));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
	rows = gtk_tree_selection_get_selected_rows(selection, &model);
	
	ptr = rows;
	
	/* Create tree row references to all of the selected rows */
	while(ptr != NULL) {
		ref = gtk_tree_row_reference_new(model, (GtkTreePath*) ptr->data);
		references = g_list_prepend(references, gtk_tree_row_reference_copy(ref));
		gtk_tree_row_reference_free(ref);
		ptr = ptr->next;
	}

	/* Remove each of the selected rows pointed to by the row reference */
	g_list_foreach(references, (GFunc) beginAccountRemoval, accounts);
	
	/* Free the tree paths, tree row references and lists */
	g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(references);
	g_list_free(rows);
}

/* Delete a account from the database */
static void beginAccountRemoval(GtkTreeRowReference *ref, intrackAccounts *accounts) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

    gtk_tree_model_get(model, &iter, ACCOUNT, &whereData, -1); /* account column */

	/* Update the database */
	gchar *query;

	query = g_strconcat("DELETE FROM `", ACCOUNTS_TABLES, "`", " WHERE number='", whereData, "'", NULL);
	databaseQuery(query, mysqlDatabase, accounts->mainWindow);
	g_free(query);

	query = g_strconcat("DROP TABLE `", whereData, "`", NULL);
	gchar *databaseSelect = g_strconcat(mysqlDatabase, "Accounts", NULL);
	databaseQuery(query, databaseSelect, accounts->mainWindow);
	g_free(query), g_free(databaseSelect);

	/* Remove the row from the tree */
	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

	g_free(whereData);    
}

/* Feed query commands to the database */
/* Good for sending updating, creation and deletion commands. Do not use for querying results */
static int databaseQuery(gchar *query, gchar *database, GtkWidget *window) {
	int query_state;	
	
	if(connectToServer() == 1) {
		return 1;
	}
	
	query_state = mysql_select_db(connection, database);	
	query_state=mysql_query(connection, query);
	
	// Failed to update the data
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, window);
		g_free(errorMessage);
				
		mysql_close(connection);
		return 1;
	}		
	
	return 0;
}

/* Updates a item in the inventory database. */
static int databaseEditItem(gchar *mysqlDatabase, gchar *inventoryTable, gchar *barcode, gchar *newCode, gchar *column) {
	
	gchar *query_string;
	int query_state;

	if(connectToServer() == 1) {
		return 1;
	}
	
	// Select the database.
	query_string = g_strconcat(mysqlDatabase, NULL);
	query_state = mysql_select_db(connection, query_string);

	//g_print("%s\n", query_string);
		
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		g_free(query_string);
		mysql_close(connection);
		
		return 1;
	}
	
	g_free(query_string);
	query_string = g_strconcat("UPDATE `", inventoryTable, "`", " SET ", column, "='", newCode, "' WHERE accountNo='", barcode, "'", NULL);

	//g_print("%s\n", query_string);

	query_state=mysql_query(connection, query_string);
	
	/* Failed to update the data */
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_UPDATING_INVENTORY, NULL);
				
		g_free(query_string);	
		mysql_close(connection);

		return 1;
	}
	
	g_free(query_string);	
	mysql_close(connection);			
	
	return 0;	
}

static void calculateTreeTotals(GtkListStore *store, intrackAccounts *accounts) {
	GtkTreeIter iter;
	gchar *rowtext;
	//gchar *coststext, **splitcosts, *joincosts;
	//gchar *profitstext, **splitprofits, *joinprofits;
	int numberOfAccounts = 0;
	float partSales = 0;

	/* Calculate the total sales, and the total sales amount in the latest tree starting from the very first row in the tree */
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)) {
		/* Number Of Accounts */
		numberOfAccounts = numberOfAccounts + 1;
		
		/* Parts Sales */
		/*
		gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, SALES, &rowtext, -1);
		partSales = partSales + atof(rowtext);
		g_free(rowtext);
		*/
		while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter)) {
			/* Parts Sales */
			/*
			gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, SALES, &rowtext, -1);
			partSales = partSales + atof(rowtext);
			g_free(rowtext);
			*/
			/* Number Of Accounts */
			numberOfAccounts++;
		}
	}

        char *locale;

        //
        locale = setlocale(LC_NUMERIC, "en_US.iso88591");
       // printf("%'d\n", 12345);
        setlocale(LC_NUMERIC, locale);
        
	gchar *numberOfAccountsChar = NULL;
	gchar *partSalesChar = NULL;
	
	numberOfAccountsChar = g_strconcat("Accounts: ", g_strdup_printf("%i", numberOfAccounts), NULL);
	//partSalesChar = g_strconcat("Total Sales: ", g_strdup_printf("%.2f", partSales), NULL);
	partSalesChar = g_strconcat("Total Sales: * ", NULL);
	
	gtk_label_set_text(GTK_LABEL(accounts->totalAccountsLabel), numberOfAccountsChar);
	gtk_label_set_text(GTK_LABEL(accounts->totalSalesLabel), partSalesChar);

	g_free(numberOfAccountsChar), g_free(partSalesChar);
}

/* Prepare to pull the account data out of the database */
static void getAccounts(GtkWidget *widget, intrackAccounts *accounts) {
	GtkListStore *store;

	store = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	
	if(strlen(gtk_entry_get_text(GTK_ENTRY(accounts->searchEntry))) > 2) {
		gchar *searchString;	
		
		searchString = g_strdup(gtk_entry_get_text(GTK_ENTRY(accounts->searchEntry)));
		pullAccounts(store, accounts, searchString);

		g_free(searchString);
		gtk_entry_set_text(GTK_ENTRY(accounts->searchEntry), "");
	}
	else if(strlen(gtk_entry_get_text(GTK_ENTRY(accounts->searchEntry))) > 0 && strlen(gtk_entry_get_text(GTK_ENTRY(accounts->searchEntry))) < 3) {
		printMessage(ERROR_SEARCH_TERMS, accounts->mainWindow);
		gtk_entry_set_text(GTK_ENTRY(accounts->searchEntry), "");
	}
	else {
		pullAccounts(store, accounts, NULL);
	}
	
	calculateTreeTotals(store, accounts);
	gtk_tree_view_set_model(GTK_TREE_VIEW(accounts->accountTree), GTK_TREE_MODEL(store));
	g_object_unref(store);	
}

static int pullAccounts(GtkListStore *store, intrackAccounts *accounts, gchar *searchString) {
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
    query_string = g_strdup(mysqlDatabase);	
	query_state = mysql_select_db(connection, query_string);	
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, accounts->mainWindow);
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}	
	
	/* Search for the user entered query */
	if(searchString != NULL)
		query_string = g_strconcat("SELECT id, accountNo, email, name, company, address, city, province, country, code, phone, fax, creationDate, dealerAccess, usa_access FROM `", ACCOUNTS_TABLES, "` WHERE accountNo LIKE '%", searchString, "%' OR email LIKE '%", searchString, "%' OR name LIKE '%", searchString, "%' OR company LIKE '%", searchString, "%' OR address LIKE '%", searchString, "%' OR city LIKE '%", searchString, "%' OR province LIKE '%", searchString, "%' OR country LIKE '%", searchString, "%' OR code LIKE '%", searchString, "%' OR phone LIKE '%", searchString, "%' OR fax LIKE '%", searchString, "%'", NULL);
	else	
		query_string = g_strconcat("SELECT id, accountNo, email, name, company, address, city, province, country, code, phone, fax, creationDate, dealerAccess, usa_access FROM `", ACCOUNTS_TABLES, "` LIMIT 0, 1000", NULL);
		
	query_state = mysql_query(connection, query_string);
	g_free(query_string);
	
	// Failed to connect and query database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, accounts->mainWindow);
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}
	
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);	
	
	while ((row = mysql_fetch_row(result))) {
		gchar *accountNo, *email, *name, *company, *address, *city, *province, *country, *code, *phone, *fax, *creationDate;
		gboolean dealer = FALSE;
		gboolean usa_access = FALSE;
		
		for(i = 0; i < num_fields; i++) {
			if(i == 1)
				accountNo = g_strdup(row[i]);
			
			if(i == 2)
				email = g_strdup(row[i]);
			
			if(i == 3)
				name = g_strdup(row[i]);
				
			if(i == 4)
				company = g_strdup(row[i]);

			if(i == 5)
				address = g_strdup(row[i]);
				
			if(i == 6)
				city = g_strdup(row[i]);
				
			if(i == 7)
				province = g_strdup(row[i]);
			
			if(i == 8)
				country = g_strdup(row[i]);
				
			if(i == 9)
				code = g_strdup(row[i]);
				
			if(i == 10)
				phone = g_strdup(row[i]);
				
			if(i == 11)
				fax = g_strdup(row[i]);
				
			if(i == 12)
				creationDate = g_strdup(row[i]);

			if(i == 13) {
				if(atoi(row[i]) == 1)
					dealer = TRUE;
				else
					dealer = FALSE;
			}	
			
			if(i == 14) {
				if(atoi(row[i]) == 1)
					usa_access = TRUE;
				else
					usa_access = FALSE;
			}								
		}

		/* Calculate the total sales for the account */
		
		/*
		gchar *totalSales;
		gdouble totalSalesDouble = 0.00;
		totalSalesDouble = getTotalSales(accountNo, accounts->mainWindow);
		totalSales = g_strdup_printf("%.2f", totalSalesDouble);
		*/
		/* Store the results into the tree now */
		gtk_list_store_append (store, &iter);
			
		gtk_list_store_set(store, &iter, ACCOUNT, accountNo, DEALER, dealer, USA_ACCESS, usa_access, EMAIL, email, NAME, name, COMPANY, company, ADDRESS, address, CITY, city, PROVSTATE, province, COUNTRY, country, CODE, code, PHONE, phone, FAX, fax, CREATION, creationDate, -1);

		g_free(accountNo), g_free(email), g_free(name), g_free(company), g_free(address), g_free(city), g_free(province);
		g_free(country), g_free(code), g_free(phone), g_free(fax), g_free(creationDate);

		//g_free(totalSales);
	}

	mysql_free_result(result);
	mysql_close(connection);
	
	return 0;
}

/* Adds up the total sales for a individual account */
static gdouble getTotalSales(gchar *accountNo, GtkWidget *mainWindow) {
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
    //query_string = g_strdup("millerAccounts");	
    query_string = g_strdup("millerSales");	
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
	
	//query_string = g_strconcat("SELECT qty, price FROM `", accountNo, "`", NULL);
	query_string = g_strconcat("SELECT orderTotal FROM `sales` WHERE userID='", accountNo, "'", NULL);
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
		totalSales = atof(accountRow[0]);
		//totalSales = totalSales + (atoi(accountRow[0]) * atof(accountRow[1]));
	}
	
	mysql_free_result(accountResult);
	mysql_close(accountConnection);	
	
	return totalSales;
}

/* Checks a account to see if any invoices over due */
/* Need to make a separate connection for this because it gets called within another connection function pullAccounts(); */
int checkInvoices(intrackAccounts *accounts, gchar *account) {
	gchar *query_string;
	int query_state;
	int i;
	int num_fields;		
	
	MYSQL *accountConnection, accountMysql;
	MYSQL_RES *accountResult;
	MYSQL_ROW accountRow;

	mysql_init(&accountMysql);

	accountConnection = mysql_real_connect(&accountMysql,mysqlServer,mysqlUsername,mysqlPassword,mysqlDatabase,0,NULL,0);

	// taxes uses a separate connection, if connection fails, check the network connection
	if(accountConnection == NULL) {
		printf(mysql_error(accountConnection), "%d\n");
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(accountConnection));

		printMessage(errorMessage, accounts->mainWindow);
		mysql_close(accountConnection);
		return 1;
	}
	
    // Select the database.
    query_string = g_strconcat(mysqlDatabase, "Accounts", NULL);	
	query_state = mysql_select_db(accountConnection, query_string);
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(accountConnection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(accountConnection));
		printMessage(errorMessage, accounts->mainWindow);
		g_free(errorMessage);
		
		mysql_close(accountConnection);
		return 1;
	}		
	
	query_string = g_strconcat("SELECT id, invoice, invoiceDate, terms, paid FROM `", account, "`", NULL);
	query_state = mysql_query(accountConnection, query_string);
	g_free(query_string);
	
	// Failed to connect and query database.
	if (query_state != 0) {
		printf(mysql_error(accountConnection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(accountConnection));
		printMessage(errorMessage, accounts->mainWindow);
		g_free(errorMessage);
		
		mysql_close(accountConnection);
		
		return 1;
	}	
	
	accountResult = mysql_store_result(accountConnection);
	num_fields = mysql_num_fields(accountResult);	
	int intDue = 0;
	
	while((accountRow = mysql_fetch_row(accountResult))) {
		gchar *invoice, *invoiceDate, *terms;
		gboolean paid = FALSE;

		for(i = 0; i < num_fields; i++) {
			if(i == 1)
				invoice = g_strdup(accountRow[i]);
			
			if(i == 2)
				invoiceDate = g_strdup(accountRow[i]);
			
			if(i == 3)
				terms = g_strdup(accountRow[i]);
				
			if(i == 4) {
				if(atoi(accountRow[i]) == 1)
					paid = TRUE;
				else
					paid = FALSE;
			}
		}
		
		/* Processing The Dates */
		GDate 	*dateTemp, *dateToday;
		int 	yearScan, monthScan, dayScan;
		gint 	overDue;
		gchar	dueDateBuffer[256];
		
		dateTemp = g_date_new();
		dateToday = g_date_new();
		
		g_date_set_time_t(dateToday, time(NULL)); // Get today's date.
				
		sscanf(invoiceDate, "%d-%d-%d", &yearScan, &monthScan, &dayScan);
		gchar *dateStrTemp = g_strdup_printf("%d-%d-%d", yearScan, monthScan, dayScan);

		g_date_set_parse(dateTemp, dateStrTemp);
		g_date_add_days(dateTemp, atoi(terms));
		g_date_strftime(dueDateBuffer, 256, "%Y-%m-%d", dateTemp);
		
		overDue = g_date_compare(dateToday, dateTemp);
		
		g_free(invoice), g_free(invoiceDate), g_free(terms);
		g_date_free(dateTemp), g_date_free(dateToday), g_free(dateStrTemp);

		if(overDue >= 0 && !paid) {
			intDue = 2;
			break;
		}
	}
	
	mysql_free_result(accountResult);
	mysql_close(accountConnection);	
	
	return intDue;
}

// Sets discontinued status on a item.
static int cellClickedDealer(GtkCellRendererToggle *render, gchar *path, intrackAccounts *accounts) {
	
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
    gboolean 		value;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ACCOUNT, &whereData, -1);	
    gtk_tree_model_get(model, &iter, DEALER, &value, -1);

	gchar *newSerChar;
		
	if(value == FALSE)
		newSerChar = g_strdup_printf("%i", 1);
	else
		newSerChar = g_strdup_printf("%i", 0);
		
	/* Update the database with the new barcode */
	databaseEditItem("millerparts", "accounts", whereData, newSerChar, "dealerAccess");

	/* Update the tree cell with the new data */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, DEALER, !value, -1);
		
	g_free(newSerChar);
    g_free(whereData);
	
	return 0;
}

// Sets USA bypass access or not.
static int cellClickedUSA(GtkCellRendererToggle *render, gchar *path, intrackAccounts *accounts) {
	
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
    gboolean 		value;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ACCOUNT, &whereData, -1);	
    gtk_tree_model_get(model, &iter, USA_ACCESS, &value, -1);

	gchar *newSerChar;
		
	if(value == FALSE)
		newSerChar = g_strdup_printf("%i", 1);
	else
		newSerChar = g_strdup_printf("%i", 0);
		
	/* Update the database with the new barcode */
	databaseEditItem("millerparts", "accounts", whereData, newSerChar, "usa_access");

	/* Update the tree cell with the new data */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, USA_ACCESS, !value, -1);
		
	g_free(newSerChar);
    g_free(whereData);
	
	return 0;
}

/* Create the tree view and columns */
static void setupAccountTree(intrackAccounts *accounts) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Account #", renderer, "text", ACCOUNT, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, ACCOUNT); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_toggle_new ();
	column = gtk_tree_view_column_new_with_attributes("Dealer", renderer, "active", DEALER, NULL);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, DEALER);
	g_signal_connect (G_OBJECT (renderer), "toggled", G_CALLBACK (cellClickedDealer), (gpointer) accounts);
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_toggle_new ();
	column = gtk_tree_view_column_new_with_attributes("USA Bypass", renderer, "active", USA_ACCESS, NULL);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, DEALER);
	g_signal_connect (G_OBJECT (renderer), "toggled", G_CALLBACK (cellClickedUSA), (gpointer) accounts);
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);	
		
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Email", renderer, "text", EMAIL, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, EMAIL); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", NAME, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, NAME); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Company", renderer, "text", COMPANY, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, COMPANY); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Address", renderer, "text", ADDRESS, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, ADDRESS); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("City", renderer, "text", CITY, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, CITY); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Province", renderer, "text", PROVSTATE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, PROVSTATE); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Country", renderer, "text", COUNTRY, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, COUNTRY); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);	

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Postal Code", renderer, "text", CODE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, CODE); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Phone", renderer, "text", PHONE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, PHONE); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Fax", renderer, "text", FAX, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, FAX); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	/*
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Sales", renderer, "text", SALES, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SALES); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);	
	*/
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Creation Date", renderer, "text", CREATION, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, CREATION); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);	

}

/* Generates a random account number */
static void generateAccountNumber(GtkWidget *widget, accountsCreate *newAccount) {
/*
#ifdef UNIX
#define seedr(a) srand48(a)
#define randm()  lrand48()
#else
#define seedr(a) srand(a)
#define randm()  rand()
#endif
*/	
	const char *charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	int len=10;
	int i;
	char * out;
	
	//seedr(time(NULL));
	srand(time(NULL));

	out = malloc(len);

	for (i = 0; i < len; i++)
		out[i] = charset[rand() % strlen(charset)]; // out[i] = charset[randm() % strlen(charset)];
	
	// Set the end of the array to terminate as NULL
	out[len] = 0;
	
	gtk_entry_set_text(GTK_ENTRY(newAccount->accountEntry), out);
}

/* Insert the data into the database */
static int sendData(GtkWidget *widget, intrackAccounts *accounts) {
	gchar *accountNumber = g_strdup(gtk_entry_get_text(GTK_ENTRY(accounts->newAccount->accountEntry)));
	
	if(strlen(accountNumber) < 1) {
		printMessage("ERROR: Account number entry is required.", accounts->newAccount->window);
		g_free(accountNumber);
		return 1;
	}
	else if(checkAccountExist(accountNumber)) {
		printMessage("ERROR: Account number already exists.", accounts->newAccount->window);
		g_free(accountNumber);
		return 1;
	}
	
	g_free(accountNumber);
	
	gchar *query_string;
	int query_state;

	/* Open MYSQL connection to the database */
	if(connectToServer() == 1) {
		return 1;
	}
	
    // Select the database.
    query_string = g_strdup(mysqlDatabase);
	query_state = mysql_select_db(connection, query_string);	
	g_free(query_string);

	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, accounts->mainWindow);
		g_free(errorMessage);		
		
		mysql_close(connection);
		return 1;
	}
	
	gchar *limitChar = g_strdup_printf("%.2f", gtk_spin_button_get_value(GTK_SPIN_BUTTON(accounts->newAccount->limitSpin)));
	gboolean toggleHold = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(accounts->newAccount->holdCheckButton));
	gchar *holdChar;
	
	if(toggleHold == TRUE)
		holdChar = g_strdup_printf("1");
	else
		holdChar = g_strdup_printf("0");
		
	query_string = g_strconcat("INSERT INTO `", ACCOUNTS_TABLES, "` (number, name, contact, address, city, provstate, code, country, phone, fax, email, hold, `limit`) VALUES ('", gtk_entry_get_text(GTK_ENTRY(accounts->newAccount->accountEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(accounts->newAccount->nameEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(accounts->newAccount->contactEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(accounts->newAccount->addressEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(accounts->newAccount->cityEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(accounts->newAccount->provinceEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(accounts->newAccount->codeEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(accounts->newAccount->countryEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(accounts->newAccount->phoneEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(accounts->newAccount->faxEntry)), "', '", gtk_entry_get_text(GTK_ENTRY(accounts->newAccount->emailEntry)), "', '", holdChar, "', '", limitChar, "')", NULL);
	
	/* Insert the data into the table */
	query_state = mysql_query(connection, query_string);
	g_free(limitChar), g_free(holdChar), g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		//printMessage(ERROR_DATABASE, accounts->newAccount->window);
		
		gchar *errorMessage = g_strdup_printf("%s\n%s", ERROR_DATABASE, mysql_error(connection));
		printMessage(errorMessage, accounts->newAccount->window);
		g_free(errorMessage);
				
		mysql_close(connection);
		return 1;
	}
	
	// Select the accounts database.
	query_string = g_strconcat(mysqlDatabase, "Accounts", NULL);
	query_state = mysql_select_db(connection, query_string);
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		//printMessage(ERROR_DATABASE, accounts->newAccount->window);
		
		gchar *errorMessage = g_strdup_printf("%s\n%s", ERROR_DATABASE, mysql_error(connection));
		printMessage(errorMessage, accounts->newAccount->window);
		g_free(errorMessage);
				
		mysql_close(connection);
		return 1;
	}
	
	/* Create the a new table in the accounts database. */
	query_string = g_strconcat("CREATE TABLE `", gtk_entry_get_text(GTK_ENTRY(accounts->newAccount->accountEntry)), "` (id int AUTO_INCREMENT, PRIMARY KEY (id), invoice varchar(100), invoiceDate timestamp default current_timestamp, terms int(11), paid int(1), amountDue double(15,2) )", NULL);
	query_state = mysql_query(connection, query_string);
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		//printMessage(ERROR_DATABASE, accounts->newAccount->window);
		
		gchar *errorMessage = g_strdup_printf("%s\n%s", ERROR_DATABASE, mysql_error(connection));
		printMessage(errorMessage, accounts->newAccount->window);
		g_free(errorMessage);
				
		mysql_close(connection);
		return 1;
	}	
	
	mysql_close(connection);
	
	closeWindow(NULL, accounts->newAccount->window);
	getAccounts(NULL, accounts); /* Refresh the accounts page */
	
	return 0;
}

/* Checks if a account exists already. Return 1 if it does not exist. Return 0 if it exists) */
static int checkAccountExist(gchar *accountNumber) {
	/* Connection Error */
	if(connectToServer() == 1)
		return 0;

	gchar *query_string = g_strconcat("SELECT number FROM ", ACCOUNTS_TABLES, " WHERE number LIKE '%", accountNumber, "%'", NULL);
	
	int query_state;
	query_state = mysql_query(connection, query_string);
	
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
	}
	
	g_free(query_string);

	//If the connection is successful and the query returns a result then the next step is to display those results:
	result = mysql_store_result(connection);
	
	// If the item is in the database, then stop and return back.
	if(mysql_fetch_row(result)) {
		/* Found item, so return back 1 */
		mysql_free_result(result); // Free up some memory.
		mysql_close(connection);
	
		return 1;
	}
	mysql_free_result(result); // Free up some memory.
	mysql_close(connection);	
	return 0;
}

/* Updates a account in the database. */
static int accountsEditItem(intrackAccounts *accounts, gchar *accountNumber, gchar *newData, gchar *column) {
	
	gchar *query_string;
	int query_state;	
	
	if(connectToServer() == 1) {
		return 1;
	}
	
	// Select the database.
	query_string = g_strdup(mysqlDatabase);
	query_state = mysql_select_db(connection, query_string);
	g_free(query_string);
		
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		//printMessage(ERROR_DATABASE, NULL);
		
		gchar *errorMessage = g_strdup_printf("%s\n%s", ERROR_DATABASE, mysql_error(connection));
		printMessage(errorMessage, accounts->mainWindow);
		g_free(errorMessage);		
		
		mysql_close(connection);
		
		return 1;
	}
	
	query_string = g_strconcat("UPDATE `", ACCOUNTS_TABLES, "`", " SET ", column, "='", newData, "' WHERE number='", accountNumber, "'", NULL);
	query_state=mysql_query(connection, query_string);
	
	g_free(query_string);
		
	/* Failed to update the data */
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		//printMessage(ERROR_UPDATING_ACCOUNT, NULL);

		gchar *errorMessage = g_strdup_printf("%s\n%s", ERROR_UPDATING_ACCOUNT, mysql_error(connection));
		printMessage(errorMessage, accounts->mainWindow);
		g_free(errorMessage);	
				
		mysql_close(connection);
		return 1;
	}
	
	mysql_close(connection);			
	
	return 0;	
}

/* Mouse click event on the tree */
static gboolean treeButtonPress(GtkWidget *widget, GdkEventButton *ev, intrackAccounts *accounts) {
    GtkTreePath *path;
    
	/* if there's no path where the click occurred... */
    if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(accounts->accountTree), ev->x, ev->y, &path, NULL, NULL, NULL)) {
        return FALSE;
	}

	/* Set the sensitivity on the viewItem Button */
	g_timeout_add(1, selectionTimer, (gpointer) accounts); 	

    switch(ev->button)
    {
        case 1: /* 1 = left click */
            break;

        case 3: /* 3 = right click */
				//gtk_menu_popup(GTK_MENU(inventory->invenMenu), NULL, NULL, NULL, NULL, 3, ev->time);
            break;
    }
	
	/* Keep track of the selected item */
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
    gchar 			*account;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, ACCOUNT, &account, -1);
    
    g_free(accounts->selectedAccount);
	accounts->selectedAccount = g_strdup(account);
	g_free(account);
       
    /* free our path */
    gtk_tree_path_free(path);  

    return FALSE;	
}

static gboolean selectionTimer(gpointer data) {
	intrackAccounts *accounts = (intrackAccounts *)data;

	/* If the user selects multiple rows in the tree, then turn off the itemView button */
	if(gtk_tree_selection_count_selected_rows(accounts->selection) > 1) {
		gtk_widget_set_sensitive(accounts->viewButton, FALSE);
	}
	else if(gtk_tree_selection_count_selected_rows(accounts->selection) == 1) {
		gtk_widget_set_sensitive(accounts->viewButton, TRUE);
	}
	else {
		gtk_widget_set_sensitive(accounts->viewButton, FALSE);
	}
	
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
            		
        case GDK_Delete:
				//deleteAccountsWindow(NULL, accounts);
			break;
			
		case GDK_KP_Delete:
				//deleteAccountsWindow(NULL, accounts);
			break;                      
    }
    return FALSE;	
}

static gboolean keyPressSelection(gpointer data) {
	intrackAccounts *accounts = (intrackAccounts *)data;

	if(gtk_tree_selection_count_selected_rows(accounts->selection) == 0) {
		gtk_widget_set_sensitive(accounts->viewButton, FALSE);
	}
	else if(gtk_tree_selection_count_selected_rows(accounts->selection) > 1) {
		gtk_widget_set_sensitive(accounts->viewButton, FALSE);
	}
	else {
		gtk_widget_set_sensitive(accounts->viewButton, TRUE);
		
		GtkTreeSelection *selection;
		GtkTreeRowReference *ref;
		GtkTreeModel *model;
		GList *rows, *ptr, *references = NULL;

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(accounts->accountTree));
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
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
		
		
	}
	return FALSE;
}

/* Get the selected row from a key press (part #2) */
static void keyPressGetRow(GtkTreeRowReference *ref, intrackAccounts *accounts) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *account;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

    gtk_tree_model_get(model, &iter, ACCOUNT, &account, -1);
    
    g_free(accounts->selectedAccount);
    accounts->selectedAccount = g_strdup(account);
    g_free(account);
    
    gtk_tree_path_free(path);
}

static void destroyGTKWidget(GtkWidget *widget, gpointer parentWindow) {
	gtk_widget_destroy(GTK_WIDGET(parentWindow));
}

static void hideGtkWidget(GtkWidget *widget, gpointer data) {
	gtk_widget_hide_all(GTK_WIDGET(data));
}

/* Loads up the account editor into memory and frees it after. A separate loading module. checkout_submit.c uses this in the loadAccount() function. */
void loadModuleAccounts(GtkWidget *widget, GtkWidget *window) {
	intrackAccounts *accounts;
	accounts = (intrackAccounts*) g_malloc (sizeof (intrackAccounts));
	accounts->selectedAccount = g_strdup(NULL);
	
	GtkBuilder *builder;
    builder = gtk_builder_new();

    // Load UI from file.
	gtk_builder_add_from_file(builder, ACCOUNTS_EDITOR_FILE, NULL);	
	
	accounts->mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "accountsWindow"));
	gtk_window_set_title(GTK_WINDOW(accounts->mainWindow), "Intrack Accounts");
	gtk_window_set_position(GTK_WINDOW(accounts->mainWindow), GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(accounts->mainWindow), GTK_WINDOW(window));
	gtk_window_set_modal(GTK_WINDOW(accounts->mainWindow), TRUE);
	g_signal_connect(G_OBJECT(accounts->mainWindow), "destroy", G_CALLBACK(freeModuleMemory), accounts);
	
    /* Exit program button from file / quit. */
    GtkWidget	*exitAccount;
    exitAccount = GTK_WIDGET(gtk_builder_get_object(builder, "accountsCloseButton"));
   	g_signal_connect(G_OBJECT(exitAccount), "activate", G_CALLBACK(destroyModuleWindow), accounts->mainWindow);	

	/* Toolbar buttons */
	/* View button */
	accounts->viewButton = GTK_WIDGET(gtk_builder_get_object(builder, "viewAccountButton"));
	g_signal_connect(G_OBJECT(accounts->viewButton), "clicked", G_CALLBACK(prepareViewAccount), accounts);
   	gtk_widget_set_sensitive(accounts->viewButton, FALSE);
   	
	/* Close the window */
    GtkWidget	*accountCloseButton;
    accountCloseButton = GTK_WIDGET(gtk_builder_get_object(builder, "closeButton"));
   	g_signal_connect(G_OBJECT(accountCloseButton), "clicked", G_CALLBACK(destroyModuleWindow), accounts->mainWindow);
   	
	/* Setup the accounts tree */
	accounts->accountContainer = GTK_WIDGET(gtk_builder_get_object(builder, "accountScroll"));
	accounts->accountTree = gtk_tree_view_new();
	setupAccountTree(accounts);
	gtk_container_add(GTK_CONTAINER(accounts->accountContainer), accounts->accountTree);
	
	/* Setup the account tree to be multi selection */
	accounts->selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (accounts->accountTree));
	gtk_tree_selection_set_mode(accounts->selection, GTK_SELECTION_MULTIPLE);	
	
	/* Setup the search entry & button */
	accounts->searchEntry = GTK_WIDGET(gtk_builder_get_object(builder, "searchEntry"));
	g_signal_connect(G_OBJECT(accounts->searchEntry), "activate", G_CALLBACK(getAccounts), accounts);
	
	GtkWidget *searchButton = GTK_WIDGET(gtk_builder_get_object(builder, "searchButton"));
	g_signal_connect(G_OBJECT(searchButton), "clicked", G_CALLBACK(getAccounts), accounts);
	
	/* Setup keypress signals on the tree */
	g_signal_connect(accounts->accountTree, "button-press-event", G_CALLBACK(treeButtonPress), accounts);
	g_signal_connect(accounts->accountTree, "key-press-event", G_CALLBACK(treeKeyPress), accounts);
	
	/* Load the inventory tree with all data from the database */
	//getAccounts(NULL, accounts);
	g_signal_connect(accounts->mainWindow, "show", G_CALLBACK(getAccounts), accounts);
	
	gtk_widget_show_all(accounts->mainWindow);

	g_object_unref(G_OBJECT(builder));	
}

static void destroyModuleWindow(GtkWidget *widget, GtkWidget *window) {
	gtk_widget_destroy(GTK_WIDGET(window));
}

static void freeModuleMemory(GtkWidget *widget, intrackAccounts *accounts) {
	if(widget) {
		destroyModuleWindow(NULL, accounts->mainWindow);
		g_free(accounts->selectedAccount);
		g_free(accounts);
	}
	else {
		g_free(accounts->selectedAccount);
		g_free(accounts);
	}
}
