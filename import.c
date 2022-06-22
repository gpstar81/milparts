//      import.c
//      Copyright 2010 Michael Rajotte <michael@michaelrajotte.com>
// 		Imports the inventory database from a csv file.

#include <mysql.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <time.h>
#include <gdk/gdkkeysyms.h>
#include "import.h"
#include "settings.h"
#include "messages.h"

/*
TODO:
(1). Modify code is it can detect UPC and EAN codes and get the product info, manufacturer, country, weight, price, etc from the barcode. (User selectable).
(2). The user selects which columns of the gtk tree to match the database columns before press OK to import.
(3). Check for memory leaks.
*/

/* fopen reference *********
txt binary  description
r 	rb 		open for reading. beginning
w 	wb 		open for writing (creates file if it doesn't exist). Deletes content and overwrites the file. beginning
a 	ab 		open for appending (creates file if it doesn't exist). end
r+ 	rb+ 	r+b 	open for reading and writing. beginning
w+ 	wb+ 	w+b 	open for reading and writing. Deletes content and overwrites the file. beginning
a+ 	ab+ 	a+b 	open for reading and writing (append if file exists). end

Special formatting for fprint and printf. 
\n is for a new line 
\" is for printing a quotation

*/

int importDatabase(GtkWidget *parentWindow) {
	
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new ("Open File",
				      GTK_WINDOW(parentWindow),
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

    gtk_filter_add(dialog, ".CSV (Comma Separated Value File)", "*.csv");
    gtk_filter_add(dialog, ".TXT (Plain Text File)", "*.txt"); 

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gtk_widget_destroy (dialog);
		importData(filename, parentWindow);
		
		g_free(filename);
	}
	else {
		gtk_widget_destroy (dialog);
	}
	
	return 0;
}

/* parse the csv file line by line into a array */
static void parseCSV_OLD(char *record, char *delim, char dataArray[][MAXFLDSIZE], int *fieldCounter) {
	int i=0;
	char *p=strtok(record, delim);
	
	while(p) {
		//g_print("%s\n", p);

		/* Strip any white space at the end of the string. Helps with making the tree look nice if any white spaces in text */
		p = g_strchomp(p);
		
		/* \" */ // for "
		gchar **dataSplit;
		gchar *dataJoin;

		/* Split the string by any encased "" text delimiters */
		dataSplit = g_strsplit_set(p, "\"", -1);
		dataJoin = g_strconcat(dataSplit[1], NULL);
		
		if(dataJoin != NULL) {
			strcpy(dataArray[i], dataJoin);
			//g_print("split\n");
		}
		else {
			strcpy(dataArray[i], p);
			//g_print("not split\n");
		}

		//strcpy(dataArray[i], p);

		g_strfreev(dataSplit);
		g_free(dataJoin);				
		
		i++;
		p=strtok('\0', delim);
	}

	*fieldCounter=i;
}

/* parse the csv file line by line into a array */
static void parseCSV(char *record, char *delim, char dataArray[][MAXFLDSIZE], int *fieldCounter) {

	gchar **stringSplit;
	
	stringSplit = g_strsplit_set(record, ";", -1);
	
	int length = g_strv_length(stringSplit);
	int i=0;
	int i2;
	
	/* Check for memory leaks */
	for(i2=0; i2 <= length; i2++) {
		
		if(stringSplit[i2] != NULL) {
			stringSplit[i2] = g_strchomp(stringSplit[i2]);

			gchar **dataSplit;
			gchar *dataJoin;

			/* Split the string by any encased "" text delimiters */
			dataSplit = g_strsplit_set(stringSplit[i2], "\"", -1);

			if(dataSplit && g_strv_length(dataSplit) >= 0)
				dataJoin = g_strconcat(dataSplit[1], NULL);		
		
			if(dataJoin != NULL) {
				strcpy(dataArray[i2], dataJoin);
				g_free(dataJoin);
			}
			else {
				strcpy(dataArray[i2], stringSplit[i2]);
			}

			g_strfreev(dataSplit);
			
			i++;
		}
	}
	
	g_strfreev(stringSplit);

	*fieldCounter=i;
}

/* open the csv file and start to parse it */
/* Create the window widget and tree */
static int importData(gchar *fileName, GtkWidget *parentWindow) {
	GtkBuilder *builder;
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, IMPORT_FILE, NULL);

	ImportData *importData;
	//importData = (ImportData*) g_malloc (sizeof (ImportData)); // Allocate memory for the data.
	importData = g_slice_new(ImportData);
	
	importData->idFlag = FALSE;
	importData->partNoFlag = FALSE;
	importData->descriptionFlag = FALSE;
	importData->manufacturerFlag = FALSE;	
	importData->replacementFlag = FALSE;
	importData->costFlag = FALSE;
	importData->costAvgFlag = FALSE;
	importData->priceFlag = FALSE;
	importData->stockFlag = FALSE;
	importData->lowStockLvlFlag = FALSE;
	importData->lastSoldFlag = FALSE;

	/* Setup the import window */
	importData->mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "importWindow"));
    gtk_window_set_title (GTK_WINDOW(importData->mainWindow), "CSV Import");
	gtk_window_set_modal(GTK_WINDOW(importData->mainWindow), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(importData->mainWindow), TRUE);
	gtk_widget_set_size_request(importData->mainWindow, 924, 632);
	gtk_window_set_deletable(GTK_WINDOW(importData->mainWindow), FALSE);
	gtk_window_set_position(GTK_WINDOW(importData->mainWindow), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(importData->mainWindow), 0);
	gtk_window_set_transient_for(GTK_WINDOW(importData->mainWindow), GTK_WINDOW(parentWindow));
	g_signal_connect(importData->mainWindow, "destroy", G_CALLBACK(freeMemory), importData);
	
	/* Setup the buttons */
	importData->importButton = GTK_WIDGET(gtk_builder_get_object(builder, "importButton"));
	g_signal_connect(G_OBJECT(importData->importButton), "clicked", G_CALLBACK(importRows), importData);
	gtk_widget_set_sensitive(importData->importButton, FALSE);
	
	importData->deleteButton = GTK_WIDGET(gtk_builder_get_object(builder, "removeButton"));
	g_signal_connect(G_OBJECT(importData->deleteButton), "clicked", G_CALLBACK(removeRows), importData);
	gtk_widget_set_sensitive(importData->deleteButton, FALSE);
	
	GtkWidget	*cancelButton;
	cancelButton = GTK_WIDGET(gtk_builder_get_object(builder, "closeButton"));
	g_signal_connect(G_OBJECT(cancelButton), "clicked", G_CALLBACK(destroyWindow), importData->mainWindow);
	
	importData->fieldCounter = 0;
	importData->recordCounter = 0;

	FILE *file=fopen(fileName,"r");

	if(file==NULL) {
		g_print("File open error\n");
		printMessage("Error opening file", parentWindow);
		destroyWindow(NULL, importData->mainWindow);
		g_object_unref(G_OBJECT(builder));
		return 1;
	}
	
	GtkListStore 	*store;
	GtkTreeIter 	iter;

	int createCounter = 0;
	int columnCounter = 0;
	/* read each row/line */
	while(fgets(importData->tmp, sizeof(importData->tmp), file) !=0) {
		int i=0;
		importData->recordCounter++;

		//printf("Record number: %d\n", importData->recordCounter);
		/* Parse and split up each row/line */
		parseCSV(importData->tmp, ";", importData->dataArray, &importData->fieldCounter);
		//importData->fieldCounter--; // Must subtract the blank end off the array. Empty data.
		
		/* Only setup the tree on the first round of the loop */
		if(createCounter == 0) {
			int storeCount = 0;
			
			/* Create a dynamic array with fieldCounter result for the number of elements */
			gsize array_size = sizeof( GType ) * importData->fieldCounter;
			GType *types = g_slice_alloc( array_size );

			/* create my store count types dynamically */
			for(storeCount=0; storeCount < importData->fieldCounter; storeCount++)
				types[storeCount] = G_TYPE_STRING;
			
			/* Create the store */
			store = gtk_list_store_newv(importData->fieldCounter, types);
			
			g_slice_free1(array_size, types);
			createCounter++;
			columnCounter = importData->fieldCounter;
		}

		/* Check to make sure it is a valid csv file. If column numbers do not match */
		if(columnCounter != importData->fieldCounter) {
			printMessage("Error: not a valid .CSV file.", parentWindow);
			destroyWindow(NULL, importData->mainWindow);
			g_object_unref(G_OBJECT(builder));
			return 1;
		}

		gtk_list_store_append (store, &iter);

		/* store each field into the columns */
		for(i=0; i < importData->fieldCounter; i++) {
			//printf("\tField number: %3d=%s\n", i, importData->dataArray[i]);
			gtk_list_store_set (store, &iter, i, importData->dataArray[i], -1);
		}
	}
	
	GtkCellRenderer 	*renderer;
	GtkTreeViewColumn 	*column;
	
	/* Create the tree model with the above store data */
	importData->importTree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	importData->selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(importData->importTree));
	gtk_tree_selection_set_mode(importData->selection, GTK_SELECTION_MULTIPLE);
	
	/* Setup keypress signals on the tree */
	g_signal_connect(importData->importTree, "button-press-event", G_CALLBACK(treeButtonPress), importData);
	g_signal_connect(importData->importTree, "key-press-event", G_CALLBACK(treeKeyPress), importData);	
		
	int i2 = 0;
	/* Setup the columns for the tree */
	for(i2=0; i2 < importData->fieldCounter; i2++) {
		GtkWidget	*comboBox;
		
		gchar *columnName;
		columnName = g_strdup_printf("column%i", i2);

		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes(columnName, renderer, "text", i2, NULL);
		gtk_tree_view_column_set_resizable(column, TRUE);
		gtk_tree_view_column_set_clickable(column, TRUE);

		comboBox = gtk_combo_box_new_text();
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "-");
		gtk_combo_box_set_active (GTK_COMBO_BOX(comboBox), 0);
		gtk_widget_set_size_request(comboBox, 160, 30);

		gtk_tree_view_column_set_widget(column, comboBox);
		gtk_widget_show(comboBox);
		
		g_signal_connect(G_OBJECT(comboBox), "changed", G_CALLBACK(setFlagsTrue), importData);
		g_signal_connect(G_OBJECT(column), "clicked", G_CALLBACK(columnClicked), importData);
		
		gtk_tree_view_append_column (GTK_TREE_VIEW(importData->importTree), column);
		
		g_free(columnName);
	}	

	/* Link the tree to the viewscroll container */
	importData->importContainer = GTK_WIDGET(gtk_builder_get_object(builder, "importContainer"));
	gtk_container_add(GTK_CONTAINER(importData->importContainer), importData->importTree);

	g_object_unref(store);
	
	fclose(file);
		
	gtk_widget_show_all(importData->mainWindow);
    g_object_unref(G_OBJECT(builder));

	return 0;
}

static void destroyWindow(GtkWidget *widget, GtkWidget *mainWindow) {
	gtk_widget_destroy(GTK_WIDGET(mainWindow));
}

static void freeMemory(GtkWidget *widget, ImportData *importData) {
	if(widget) {
		destroyWindow(NULL, importData->mainWindow);
		g_free(importData);		
	}
}

static void beginImport(GtkTreeRowReference *ref, ImportData *importData) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;
	GtkTreeViewColumn *column;
	GtkWidget 		*comboBox;
	gchar	*comboText;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(importData->importTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

	int i;
	
	gboolean startBool = TRUE;
	gchar *cellData;
	gchar *startQuery = g_strconcat(NULL, NULL);
	gchar *startQueryTemp = g_strconcat(NULL, NULL);
	gchar *endQuery = g_strconcat(NULL, NULL);
	gchar *endQueryTemp = g_strconcat(NULL, NULL);
	gchar *updateQuery = g_strconcat(NULL, NULL);
	gchar *updateQueryTemp = g_strconcat(NULL, NULL);
	gchar *whereQuery;
	
	for(i = 0; i < importData->fieldCounter; i++) {
		column = gtk_tree_view_get_column(GTK_TREE_VIEW(importData->importTree), i);
		comboBox = gtk_tree_view_column_get_widget(GTK_TREE_VIEW_COLUMN(column));
		comboText = gtk_combo_box_get_active_text(GTK_COMBO_BOX(comboBox));
		
		if(!strcmp(comboText, "partNo")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);

			if(startBool == TRUE) {
				startBool = FALSE;
				g_free(startQuery);
				g_free(endQuery);
				g_free(updateQuery);

				updateQuery = g_strconcat("partNo='", cellData, "'", NULL);
				startQuery = g_strconcat("partNo", NULL);
				endQuery = g_strconcat("'", cellData, "'", NULL);
			}
			else {
				g_free(updateQueryTemp);
				updateQueryTemp = g_strconcat(updateQuery, NULL);
				g_free(updateQuery);
				updateQuery = g_strconcat(updateQueryTemp, ", partNo='", cellData, "'", NULL);
				g_free(startQueryTemp);
				startQueryTemp = g_strconcat(startQuery, NULL);
				g_free(startQuery);
				startQuery = g_strconcat(startQueryTemp, ", partNo", NULL);
				g_free(endQueryTemp);
				endQueryTemp = g_strconcat(endQuery, NULL);
				g_free(endQuery);
				endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
			}
			
			whereQuery = g_strconcat(cellData, NULL);
			g_free(cellData);
		}
		
		if(!strcmp(comboText, "description")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);

			if(startBool == TRUE) {
				startBool = FALSE;
				g_free(startQuery);
				g_free(endQuery);
				g_free(updateQuery);

				updateQuery = g_strconcat("description='", cellData, "'", NULL);								
				startQuery = g_strconcat("description", NULL);
				endQuery = g_strconcat("'", cellData, "'", NULL);
			}
			else {
				g_free(updateQueryTemp);
				updateQueryTemp = g_strconcat(updateQuery, NULL);
				g_free(updateQuery);
				updateQuery = g_strconcat(updateQueryTemp, ", description='", cellData, "'", NULL);				
				g_free(startQueryTemp);
				startQueryTemp = g_strconcat(startQuery, NULL);
				g_free(startQuery);
				startQuery = g_strconcat(startQueryTemp, ", description", NULL);
				g_free(endQueryTemp);
				endQueryTemp = g_strconcat(endQuery, NULL);
				g_free(endQuery);
				endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
			}
			
			g_free(cellData);
		}
		
		if(!strcmp(comboText, "manufacturer")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);

			if(startBool == TRUE) {
				startBool = FALSE;
				g_free(startQuery);
				g_free(endQuery);
				g_free(updateQuery);

				updateQuery = g_strconcat("manufacturer='", cellData, "'", NULL);								
				startQuery = g_strconcat("manufacturer", NULL);
				endQuery = g_strconcat("'", cellData, "'", NULL);
			}
			else {
				g_free(updateQueryTemp);
				updateQueryTemp = g_strconcat(updateQuery, NULL);
				g_free(updateQuery);
				updateQuery = g_strconcat(updateQueryTemp, ", manufacturer='", cellData, "'", NULL);				
				g_free(startQueryTemp);
				startQueryTemp = g_strconcat(startQuery, NULL);
				g_free(startQuery);
				startQuery = g_strconcat(startQueryTemp, ", manufacturer", NULL);
				g_free(endQueryTemp);
				endQueryTemp = g_strconcat(endQuery, NULL);
				g_free(endQuery);
				endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
			}
			
			g_free(cellData);
		}		
		
		if(!strcmp(comboText, "replacement part")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);

			if(startBool == TRUE) {
				startBool = FALSE;
				g_free(startQuery);
				g_free(endQuery);	
				g_free(updateQuery);

				updateQuery = g_strconcat("replacePart='", cellData, "'", NULL);							
				startQuery = g_strconcat("replacePart", NULL);
				endQuery = g_strconcat("'", cellData, "'", NULL);
			}
			else {
				g_free(updateQueryTemp);
				updateQueryTemp = g_strconcat(updateQuery, NULL);
				g_free(updateQuery);
				updateQuery = g_strconcat(updateQueryTemp, ", replacePart='", cellData, "'", NULL);				
				g_free(startQueryTemp);
				startQueryTemp = g_strconcat(startQuery, NULL);
				g_free(startQuery);
				startQuery = g_strconcat(startQueryTemp, ", replacePart", NULL);
				g_free(endQueryTemp);
				endQueryTemp = g_strconcat(endQuery, NULL);
				g_free(endQuery);
				endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
			}
			
			g_free(cellData);
		}
		
		if(!strcmp(comboText, "cost")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);
			
			// Only update cost if the new number is greater than 0.
			if(atof(cellData) > 0.01) {
				if(startBool == TRUE) {
					startBool = FALSE;
					g_free(startQuery);
					g_free(endQuery);	
					g_free(updateQuery);
				
					updateQuery = g_strconcat("cost='", cellData, "'", NULL);							
					startQuery = g_strconcat("cost", NULL);
					endQuery = g_strconcat("'", cellData, "'", NULL);
				}
				else {
					g_free(updateQueryTemp);
					updateQueryTemp = g_strconcat(updateQuery, NULL);
					g_free(updateQuery);
					updateQuery = g_strconcat(updateQueryTemp, ", cost='", cellData, "'", NULL);				
					g_free(startQueryTemp);
					startQueryTemp = g_strconcat(startQuery, NULL);
					g_free(startQuery);
					startQuery = g_strconcat(startQueryTemp, ", cost", NULL);
					g_free(endQueryTemp);
					endQueryTemp = g_strconcat(endQuery, NULL);
					g_free(endQuery);
					endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
				}
			}
			g_free(cellData);
		}
		
		if(!strcmp(comboText, "costAvg")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);

			if(startBool == TRUE) {
				startBool = FALSE;
				g_free(startQuery);
				g_free(endQuery);		
				g_free(updateQuery);

				updateQuery = g_strconcat("costAvg='", cellData, "'", NULL);						
				startQuery = g_strconcat("costAvg", NULL);
				endQuery = g_strconcat("'", cellData, "'", NULL);
			}
			else {
				g_free(updateQueryTemp);
				updateQueryTemp = g_strconcat(updateQuery, NULL);
				g_free(updateQuery);
				updateQuery = g_strconcat(updateQueryTemp, ", costAvg='", cellData, "'", NULL);				
				g_free(startQueryTemp);
				startQueryTemp = g_strconcat(startQuery, NULL);
				g_free(startQuery);
				startQuery = g_strconcat(startQueryTemp, ", costAvg", NULL);
				g_free(endQueryTemp);
				endQueryTemp = g_strconcat(endQuery, NULL);
				g_free(endQuery);
				endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
			}
			
			g_free(cellData);
		}
		
		if(!strcmp(comboText, "price")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);

			// Only update price if the new number is greater than 0.
			if(atof(cellData) > 0.01) {
				if(startBool == TRUE) {
					startBool = FALSE;
					g_free(startQuery);
					g_free(endQuery);	
					g_free(updateQuery);

					updateQuery = g_strconcat("price='", cellData, "'", NULL);							
					startQuery = g_strconcat("price", NULL);
					endQuery = g_strconcat("'", cellData, "'", NULL);
				}
				else {
					g_free(updateQueryTemp);
					updateQueryTemp = g_strconcat(updateQuery, NULL);
					g_free(updateQuery);
					updateQuery = g_strconcat(updateQueryTemp, ", price='", cellData, "'", NULL);				
					g_free(startQueryTemp);
					startQueryTemp = g_strconcat(startQuery, NULL);
					g_free(startQuery);
					startQuery = g_strconcat(startQueryTemp, ", price", NULL);
					g_free(endQueryTemp);
					endQueryTemp = g_strconcat(endQuery, NULL);
					g_free(endQuery);
					endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
				}
			}
			g_free(cellData);
		}
		
		if(!strcmp(comboText, "stock")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);

			if(startBool == TRUE) {
				startBool = FALSE;
				g_free(startQuery);
				g_free(endQuery);	
				g_free(updateQuery);

				updateQuery = g_strconcat("stock='", cellData, "'", NULL);							
				startQuery = g_strconcat("stock", NULL);
				endQuery = g_strconcat("'", cellData, "'", NULL);
			}
			else {
				g_free(updateQueryTemp);
				updateQueryTemp = g_strconcat(updateQuery, NULL);
				g_free(updateQuery);
				updateQuery = g_strconcat(updateQueryTemp, ", stock='", cellData, "'", NULL);				
				g_free(startQueryTemp);
				startQueryTemp = g_strconcat(startQuery, NULL);
				g_free(startQuery);
				startQuery = g_strconcat(startQueryTemp, ", stock", NULL);
				g_free(endQueryTemp);
				endQueryTemp = g_strconcat(endQuery, NULL);
				g_free(endQuery);
				endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
			}
			
			g_free(cellData);
		}
		
		if(!strcmp(comboText, "low stock level")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);

			if(startBool == TRUE) {
				startBool = FALSE;
				g_free(startQuery);
				g_free(endQuery);	
				g_free(updateQuery);

				updateQuery = g_strconcat("lowStockLvl='", cellData, "'", NULL);							
				startQuery = g_strconcat("lowStockLvl", NULL);
				endQuery = g_strconcat("'", cellData, "'", NULL);
			}
			else {
				g_free(updateQueryTemp);
				updateQueryTemp = g_strconcat(updateQuery, NULL);
				g_free(updateQuery);
				updateQuery = g_strconcat(updateQueryTemp, ", lowStockLvl='", cellData, "'", NULL);				
				g_free(startQueryTemp);
				startQueryTemp = g_strconcat(startQuery, NULL);
				g_free(startQuery);
				startQuery = g_strconcat(startQueryTemp, ", lowStockLvl", NULL);
				g_free(endQueryTemp);
				endQueryTemp = g_strconcat(endQuery, NULL);
				g_free(endQuery);
				endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
			}
			
			g_free(cellData);
		}
		
		if(!strcmp(comboText, "last sold")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);

			if(startBool == TRUE) {
				startBool = FALSE;
				g_free(startQuery);
				g_free(endQuery);		
				g_free(updateQuery);

				updateQuery = g_strconcat("lastSold='", cellData, "'", NULL);						
				startQuery = g_strconcat("lastSold", NULL);
				endQuery = g_strconcat("'", cellData, "'", NULL);
			}
			else {
				g_free(updateQueryTemp);
				updateQueryTemp = g_strconcat(updateQuery, NULL);
				g_free(updateQuery);
				updateQuery = g_strconcat(updateQueryTemp, ", lastSold='", cellData, "'", NULL);				
				g_free(startQueryTemp);
				startQueryTemp = g_strconcat(startQuery, NULL);
				g_free(startQuery);
				startQuery = g_strconcat(startQueryTemp, ", lastSold", NULL);
				g_free(endQueryTemp);
				endQueryTemp = g_strconcat(endQuery, NULL);
				g_free(endQuery);
				endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
			}
			
			g_free(cellData);
		}
		
		/*
		if(!strcmp(comboText, "serial required")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);

			if(startBool == TRUE) {
				startBool = FALSE;
				g_free(startQuery);
				g_free(endQuery);	
				g_free(updateQuery);

				updateQuery = g_strconcat("serialRequired='", cellData, "'", NULL);							
				startQuery = g_strconcat("serialRequired", NULL);
				endQuery = g_strconcat("'", cellData, "'", NULL);
			}
			else {
				g_free(updateQueryTemp);
				updateQueryTemp = g_strconcat(updateQuery, NULL);
				g_free(updateQuery);
				updateQuery = g_strconcat(updateQueryTemp, ", serialRequired='", cellData, "'", NULL);				
				g_free(startQueryTemp);
				startQueryTemp = g_strconcat(startQuery, NULL);
				g_free(startQuery);
				startQuery = g_strconcat(startQueryTemp, ", serialRequired", NULL);
				g_free(endQueryTemp);
				endQueryTemp = g_strconcat(endQuery, NULL);
				g_free(endQuery);
				endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
			}
			
			g_free(cellData);
		}
		*/
		
		/*
		if(!strcmp(comboText, "number of serials")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);
			int numSerials = atoi(cellData);
			gchar *requireSerial;
			
			if(numSerials > 0)
				requireSerial = g_strconcat("1", NULL);
			else
				requireSerial = g_strconcat("0", NULL);
			
			// Serial number max limit
			if(numSerials > 20) {
				g_free(cellData);
				cellData = g_strconcat("20", NULL);
			}
			
			if(startBool == TRUE) {
				startBool = FALSE;
				g_free(startQuery);
				g_free(endQuery);		
				g_free(updateQuery);

				updateQuery = g_strconcat("serialRequired='", requireSerial, "', numberOfSerial='", cellData, "'", NULL);						
				startQuery = g_strconcat("serialRerquired, numberOfSerial", NULL);
				endQuery = g_strconcat("'", requireSerial, "', '", cellData, "'", NULL);
			}
			else {
				g_free(updateQueryTemp);
				updateQueryTemp = g_strconcat(updateQuery, NULL);
				g_free(updateQuery);
				updateQuery = g_strconcat(updateQueryTemp, ", serialRequired='", requireSerial, "', numberOfSerial='", cellData, "'", NULL);				
				g_free(startQueryTemp);
				startQueryTemp = g_strconcat(startQuery, NULL);
				g_free(startQuery);
				startQuery = g_strconcat(startQueryTemp, ", serialRequired, numberOfSerial", NULL);
				g_free(endQueryTemp);
				endQueryTemp = g_strconcat(endQuery, NULL);
				g_free(endQuery);
				endQuery = g_strconcat(endQueryTemp, ", '", requireSerial, "', '", cellData, "'", NULL);
			}
			
			g_free(cellData), g_free(requireSerial);
		}
		*/
		/*
		if(!strcmp(comboText, "last sold")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);

			if(startBool == TRUE) {
				startBool = FALSE;
				g_free(startQuery);
				g_free(endQuery);		
				g_free(updateQuery);

				updateQuery = g_strconcat("lastSold='", cellData, "'", NULL);						
				startQuery = g_strconcat("lastSold", NULL);
				endQuery = g_strconcat("'", cellData, "'", NULL);
			}
			else {
				g_free(updateQueryTemp);
				updateQueryTemp = g_strconcat(updateQuery, NULL);
				g_free(updateQuery);
				updateQuery = g_strconcat(updateQueryTemp, ", lastSold='", cellData, "'", NULL);				
				g_free(startQueryTemp);
				startQueryTemp = g_strconcat(startQuery, NULL);
				g_free(startQuery);
				startQuery = g_strconcat(startQueryTemp, ", lastSold", NULL);
				g_free(endQueryTemp);
				endQueryTemp = g_strconcat(endQuery, NULL);
				g_free(endQuery);
				endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
			}
			
			g_free(cellData);
		}
		*/
		
		/*
		if(!strcmp(comboText, "duty")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);

			if(startBool == TRUE) {
				startBool = FALSE;
				g_free(startQuery);
				g_free(endQuery);		
				g_free(updateQuery);

				updateQuery = g_strconcat("duty='", cellData, "'", NULL);						
				startQuery = g_strconcat("duty", NULL);
				endQuery = g_strconcat("'", cellData, "'", NULL);
			}
			else {
				g_free(updateQueryTemp);
				updateQueryTemp = g_strconcat(updateQuery, NULL);
				g_free(updateQuery);
				updateQuery = g_strconcat(updateQueryTemp, ", duty='", cellData, "'", NULL);				
				g_free(startQueryTemp);
				startQueryTemp = g_strconcat(startQuery, NULL);
				g_free(startQuery);
				startQuery = g_strconcat(startQueryTemp, ", duty", NULL);
				g_free(endQueryTemp);
				endQueryTemp = g_strconcat(endQuery, NULL);
				g_free(endQuery);
				endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
			}
			
			g_free(cellData);
		}
		*/
		
		/*
		if(!strcmp(comboText, "country of origin")) {
			gtk_tree_model_get(model, &iter, i, &cellData, -1);

			if(startBool == TRUE) {
				startBool = FALSE;
				g_free(startQuery);
				g_free(endQuery);			
				g_free(updateQuery);

				updateQuery = g_strconcat("countryOrigin='", cellData, "'", NULL);					
				startQuery = g_strconcat("countryOrigin", NULL);
				endQuery = g_strconcat("'", cellData, "'", NULL);
			}
			else {
				g_free(updateQueryTemp);
				updateQueryTemp = g_strconcat(updateQuery, NULL);
				g_free(updateQuery);
				updateQuery = g_strconcat(updateQueryTemp, ", countryOrigin='", cellData, "'", NULL);				
				g_free(startQueryTemp);
				startQueryTemp = g_strconcat(startQuery, NULL);
				g_free(startQuery);
				startQuery = g_strconcat(startQueryTemp, ", countryOrigin", NULL);
				g_free(endQueryTemp);
				endQueryTemp = g_strconcat(endQuery, NULL);
				g_free(endQuery);
				endQuery = g_strconcat(endQueryTemp, ", '", cellData, "'", NULL);
			}
			
			g_free(cellData);
		}	
		*/
		g_free(comboText);
	}
	
	gchar *finalQuery;
	
	/* Not exist */
	if(checkIfExist(whereQuery) == 1)
		finalQuery = g_strconcat("INSERT INTO `", mysqlTables, "` (", startQuery, ") VALUES (", endQuery, ")", NULL);
	else
		finalQuery = g_strconcat("UPDATE `", mysqlTables, "` SET ", updateQuery, " WHERE partNo='", whereQuery, "'", NULL);

	sendData(finalQuery);
	
	//g_print("%s\n", finalQuery);

	g_free(updateQuery), g_free(updateQueryTemp);;
	g_free(finalQuery);
	g_free(whereQuery);
	g_free(startQuery), g_free(startQueryTemp);
	g_free(endQuery), g_free(endQueryTemp);
}

static int sendData(gchar *finalQuery) {
	
	MYSQL *importConnection, importMysql;
	int query_state;
	gchar *query_string;
	
	mysql_init(&importMysql);

	importConnection = mysql_real_connect(&importMysql,mysqlServer,mysqlUsername,mysqlPassword,MYSQL_DEFAULT_DATABASE, 0, 0, 0);
	
    // Select the database.
    query_string = g_strconcat(mysqlDatabase, NULL);	
	query_state = mysql_select_db(importConnection, query_string);
	
	// Failed to connect and select database.	
	if(query_state != 0) {
		printf(mysql_error(importConnection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		g_free(query_string);
		mysql_close(importConnection);

		return 1;
	}
	
	g_free(query_string);
	
	query_state = mysql_query(importConnection, finalQuery);
	
	if(query_state != 0) {
		printf(mysql_error(importConnection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		g_free(query_string);
		mysql_close(importConnection);

		return 1;
		
	}
		
	mysql_close(importConnection);
	
	return 0;
}

static void importRows(GtkWidget *widget, ImportData *importData) {
	//http://www.gtkforums.com/about3801.html
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GList *rows, *ptr, *references = NULL;
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(importData->importTree));
	gtk_tree_selection_select_all(selection);
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(importData->importTree));
	rows = gtk_tree_selection_get_selected_rows(selection, &model);
	
	ptr = rows;
	
	while(ptr != NULL) {
		ref = gtk_tree_row_reference_new(model, (GtkTreePath*) ptr->data);
		references = g_list_prepend(references, gtk_tree_row_reference_copy(ref));
		gtk_tree_row_reference_free(ref);
		ptr = ptr->next;
	}
	
	g_list_foreach(references, (GFunc) beginImport, importData);
	
	g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(references);
	g_list_free(rows);
	
	/* close the window */
	destroyWindow(NULL, importData->mainWindow);
}

/* Remove rows of data out of the tree */
static void beginRowRemoval(GtkTreeRowReference *ref, ImportData *importData) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(importData->importTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

	/* Remove the row from the tree */
	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
}

/* Get the selection of rows of data to remove out of the tree */
static void removeRows(GtkWidget *widget, ImportData *importData) {
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GList *rows, *ptr, *references = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(importData->importTree));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(importData->importTree));
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
	g_list_foreach(references, (GFunc) beginRowRemoval, importData);
	
	/* Free the tree paths, tree row references and lists */
	g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(references);
	g_list_free(rows);	
	
	gtk_widget_set_sensitive(importData->deleteButton, FALSE);
}

/* Mouse click event on the tree */
static gboolean treeButtonPress(GtkWidget *widget, GdkEventButton *ev, ImportData *importData) {

    GtkTreePath *path;
    
	/* if there's no path where the click occurred... */
    if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(importData->importTree), ev->x, ev->y, &path, NULL, NULL, NULL)) {
		gtk_widget_set_sensitive(importData->deleteButton, FALSE);
        return FALSE;
	}

	gtk_widget_set_sensitive(importData->deleteButton, TRUE);

    /* LMB */
    switch(ev->button)
    {
        case 1: /* 1 = left click */
            break;
            
        case 2: /* 2 = middle click */
			break;

        case 3: /* 3 = right click */
				//gtk_menu_popup(GTK_MENU(inventory->invenMenu), NULL, NULL, NULL, NULL, 3, ev->time);
            break;
    }
     
    /* free our path */
    gtk_tree_path_free(path);  

    return FALSE;	
}

/* Keyboard key press event on the tree */
static gboolean treeKeyPress(GtkWidget *widget, GdkEventKey *ev, ImportData *importData) {
	
    switch(ev->keyval)
    {       
        case GDK_Up:
				/* Add a 1 millisecond delay before pulling the selection from the tree to get accurate selection. */
				g_timeout_add (1, keyPressSelection, (gpointer) importData); 		
            break;

        case GDK_KP_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) importData); 		
            break;

        case GDK_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) importData); 		
            break;

        case GDK_KP_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) importData); 		
            break;  
            
        case GDK_KP_Home:
				g_timeout_add (1, keyPressSelection, (gpointer) importData); 		
            break;              
            
        case GDK_KP_End:
				g_timeout_add (1, keyPressSelection, (gpointer) importData); 		
            break;     
            
        case GDK_Page_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) importData); 		
            break;  

        case GDK_Page_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) importData); 		
            break;  
            
        case GDK_KP_Page_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) importData); 		
            break;  
            
        case GDK_KP_Page_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) importData); 		
            break;      
            
        case GDK_Home:
				g_timeout_add (1, keyPressSelection, (gpointer) importData); 		
            break;  
            
        case GDK_End:
				g_timeout_add (1, keyPressSelection, (gpointer) importData); 		
            break;
            		
        case GDK_Delete:
				removeRows(NULL, importData);
			break;
			
		case GDK_KP_Delete:
				removeRows(NULL, importData);
			break;                      
    }
	
    return FALSE;	
}

static gboolean keyPressSelection(gpointer data) {

	ImportData *importData = (ImportData *)data;

	if(gtk_tree_selection_count_selected_rows(importData->selection) == 0) {
		gtk_widget_set_sensitive(importData->deleteButton, FALSE);
	}
	else if(gtk_tree_selection_count_selected_rows(importData->selection) > 0) {
		gtk_widget_set_sensitive(importData->deleteButton, TRUE);
	}

	return FALSE;	
}

static void columnClicked(GtkWidget *column, ImportData *importData) {
	
	GtkWidget 		*comboBox;
	GtkTreeModel 	*comboTree;
	GtkTreeIter 	iter;
	gchar			*flagText;
	
	comboBox = gtk_tree_view_column_get_widget(GTK_TREE_VIEW_COLUMN(column));
	
	flagText = gtk_combo_box_get_active_text(GTK_COMBO_BOX(comboBox));
	setFlagsFalse(importData, flagText);
	
  	gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), 0);
	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(comboBox), &iter);
	
	comboTree = gtk_combo_box_get_model(GTK_COMBO_BOX (comboBox));
	
	gint iBox = 0;
	/* First we pull the tree model from the combo box and count how many items are in it */
	while(gtk_tree_model_iter_next(GTK_TREE_MODEL(comboTree), &iter)) {
		iBox++;
	}

	/* Set the default back to nothing selected in the combo box selection */
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), 0);
	
	gint iBox2 = 1;
	/* Then we remove all the items from the combo box except the first one, which is always no selection "-" */
	while(iBox2 <= iBox) {
		gtk_combo_box_remove_text(GTK_COMBO_BOX(comboBox), 1);
		iBox2++;
	}	

	iBox = 0;
	if(importData->idFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "id");
		
		iBox++;
		//if(!strcmp(flagText, "id"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);
	}
	
	if(importData->partNoFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "partNo");
		
		iBox++;
		//if(!strcmp(flagText, "barcode"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);		
	}
	
	if(importData->descriptionFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "description");
		
		iBox++;
		//if(!strcmp(flagText, "description"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);	
	}
	
	if(importData->manufacturerFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "manufacturer");
		
		iBox++;
		//if(!strcmp(flagText, "description"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);	
	}	
		
	if(importData->replacementFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "replacement part");
		
		iBox++;
		//if(!strcmp(flagText, "description"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);	
	}
	
	if(importData->costFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "cost");
		
		iBox++;
		//if(!strcmp(flagText, "manufacturer"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);
	}
	
	if(importData->costAvgFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "cost average");
		
		iBox++;
		//if(!strcmp(flagText, "category"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);	
	}
	
	if(importData->priceFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "price");
		
		iBox++;
		//if(!strcmp(flagText, "cost"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);		
	}
	
	if(importData->stockFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "stock");
		
		iBox++;
		//if(!strcmp(flagText, "cost average"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);		
	}
	
	if(importData->lowStockLvlFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "low stock level");
		
		iBox++;
		//if(!strcmp(flagText, "price"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);		
	}
	
	if(importData->lastSoldFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "last sold");
		
		iBox++;
		//if(!strcmp(flagText, "stock"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);		
	}
	
	/*
	if(importData->serialRequiredFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "serial required");
		
		iBox++;
		//if(!strcmp(flagText, "serial required"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);		
	}
	*/
	
	/*
	if(importData->numberOfSerialFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "number of serials");
		
		iBox++;
		//if(!strcmp(flagText, "number of serials"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);		
	}
	*/
	
	/*
	if(importData->lastSoldFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "last sold");
		
		iBox++;
		//if(!strcmp(flagText, "last sold"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);
	}
	*/
	
	/*
	if(importData->dutyFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "duty");
		
		iBox++;
		//if(!strcmp(flagText, "duty"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);		
	}
	*/
	
	/*
	if(importData->countryOriginFlag == FALSE) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBox), "country of origin");
		
		iBox++;
		//if(!strcmp(flagText, "country of origin"))
			//gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), iBox);		
	}
	*/
	gtk_combo_box_popup(GTK_COMBO_BOX(comboBox));
	
	g_free(flagText);
}

static void setFlagsFalse(ImportData *importData, gchar *flagText) {
	
	if(!strcmp(flagText, "id"))
		importData->idFlag = FALSE;
		
	if(!strcmp(flagText, "partNo")) {
		importData->partNoFlag = FALSE;
		gtk_widget_set_sensitive(importData->importButton, FALSE);
	}
	
	if(!strcmp(flagText, "description"))
		importData->descriptionFlag = FALSE;
		
	if(!strcmp(flagText, "manufacturer"))
		importData->manufacturerFlag = FALSE;		
		
	if(!strcmp(flagText, "replacement part"))
		importData->replacementFlag = FALSE;
		
	if(!strcmp(flagText, "cost"))
		importData->costFlag = FALSE;
		
	if(!strcmp(flagText, "cost average"))
		importData->costAvgFlag = FALSE;
		
	if(!strcmp(flagText, "price"))
		importData->priceFlag = FALSE;
		
	if(!strcmp(flagText, "stock"))
		importData->stockFlag = FALSE;
		
	if(!strcmp(flagText, "low stock level"))
		importData->lowStockLvlFlag = FALSE;
		
	if(!strcmp(flagText, "last sold"))
		importData->lastSoldFlag = FALSE;
}

static void setFlagsTrue(GtkWidget *widget, ImportData *importData) {
	
	gchar *flagText;
	flagText = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
	
	if(!strcmp(flagText, "id"))
		importData->idFlag = TRUE;
		
	if(!strcmp(flagText, "partNo")) {
		importData->partNoFlag = TRUE;
		gtk_widget_set_sensitive(importData->importButton, TRUE);
	}
	
	if(!strcmp(flagText, "description"))
		importData->descriptionFlag = TRUE;
		
	if(!strcmp(flagText, "manufacturer"))
		importData->manufacturerFlag = TRUE;
				
	if(!strcmp(flagText, "replacement part"))
		importData->replacementFlag = TRUE;
		
	if(!strcmp(flagText, "cost"))
		importData->costFlag = TRUE;
		
	if(!strcmp(flagText, "cost average"))
		importData->costAvgFlag = TRUE;
		
	if(!strcmp(flagText, "price"))
		importData->priceFlag = TRUE;
		
	if(!strcmp(flagText, "stock"))
		importData->stockFlag = TRUE;
		
	if(!strcmp(flagText, "low stock level"))
		importData->lowStockLvlFlag = TRUE;
		
	if(!strcmp(flagText, "last sold"))
		importData->lastSoldFlag = TRUE;
		
	g_free(flagText);
}

static int checkIfExist(gchar *barcode) {
	
	/* Connection Error */
	if(connectToServer() == 1)
		return 1;
	
	gchar *query_string;
	query_string = g_strconcat("SELECT partNo FROM ", mysqlTables, " WHERE partNo = '", barcode, "'", NULL);
	
	int query_state;
	query_state = mysql_query(connection, query_string);
	
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
	}
	g_free(query_string);

	//If the connection is successful and the query returns a result then the next step is to display those results:
	result = mysql_store_result(connection);
	
	// If the item is not in the database, then stop and return back. ie. results fetch row is null or empty.
	if (!mysql_fetch_row(result)) {
		mysql_free_result(result); // Free up some memory.
		mysql_close(connection);
	
		gchar *errorMessage = g_strconcat(barcode, " Item Not Found", NULL);

		//printMessage(errorMessage);
		
		g_free(errorMessage);

		return 1;
	}
	
	mysql_free_result(result); // Free up some memory.
	mysql_close(connection);	

	return 0;
}
