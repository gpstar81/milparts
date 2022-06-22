//      export.c
//      Copyright 2010-2014 Michael Rajotte <michael@michaelrajotte.com>
// 		Exports data to a .csv file.

#include <mysql.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <time.h>
#include "export.h"
#include "settings.h"
#include "messages.h"


//Exporting out the database into a csv file.
/*
int exportTablesPDF(char *fileName) {
	
	const char *font_list[] = {
		"Courier",
		"Courier-Bold",
		"Courier-Oblique",
		"Courier-BoldOblique",
		"Helvetica",
		"Helvetica-Bold",
		"Helvetica-Oblique",
		"Helvetica-BoldOblique",
		"Times-Roman",
		"Times-Bold",
		"Times-Italic",
		"Times-BoldItalic",
		"Symbol",
		"ZapfDingbats",
		NULL
	};	
	
	int num_fields;
	int i;
	gchar *query_string = NULL;
	gchar *printValue = NULL;
	
	MYSQL_FIELD *field;

	mysql_init(&mysql);

	connection = mysql_real_connect(&mysql,mysqlServer,mysqlUsername,mysqlPassword,mysqlDatabase,0,0,0);

	if (connection == NULL) {
		printf(mysql_error(connection), "%d\n");
		
		printMessage(ERROR_CONNECTION);
		
		return 1;
	}	
	
	//query_string = "SELECT barcode, name, description, price, stock from scan";
	query_string = "SELECT barcode, name, price, stock from ";
	query_string = g_strconcat(query_string, mysqlTables, NULL);
	
	mysql_query(connection, query_string);
	
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);
		
    HPDF_Doc  pdf;
    HPDF_Page page;
    HPDF_Font def_font;
    HPDF_REAL tw;
    HPDF_REAL height;
    HPDF_REAL width;
    HPDF_UINT ipdf;

    pdf = HPDF_New (NULL, NULL);	
    
    // Add a new page object
    page = HPDF_AddPage (pdf);

	HPDF_Page_SetSize (page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);    
	
    height = HPDF_Page_GetHeight (page);
    width = HPDF_Page_GetWidth (page);
    	
    // Print the title of the page (with positioning center)
    
    def_font = HPDF_GetFont (pdf, "Helvetica", NULL);
    HPDF_Page_SetFontAndSize (page, def_font, 24);
    HPDF_SetInfoAttr (pdf, HPDF_INFO_CREATOR, "Barcode Scanner Basic Edition");
    HPDF_SetInfoAttr (pdf, HPDF_INFO_PRODUCER, "Barcode Scanner Basic Edition");
    HPDF_SetInfoAttr (pdf, HPDF_INFO_AUTHOR, "Barcode Scanner Basic Edition");
    
	//
    //const char *page_title = "Database Export";
    //tw = HPDF_Page_TextWidth (page, page_title);
    //HPDF_Page_BeginText (page);
    //HPDF_Page_TextOut (page, (width - tw) / 2, height - 50, page_title); // -> centers the text to the center of the page.
    //HPDF_Page_EndText (page);
	//
	
	// text output2
    HPDF_Page_BeginText (page); // --> Start Text Output

    HPDF_Page_SetFontAndSize (page, def_font, 10);
    HPDF_Page_MoveTextPos (page, 20, height - 20);
	// EOF text output2
	
	int pdfCounter = 0;
	int fieldsFound = 0; // -> used for pdf spacing output, so extra white space isn't issued when it loops through to check for field names on each row.
	
	// Write the database data into a pdf.
	while ( ( row = mysql_fetch_row(result)) ) {
		
		if(pdfCounter >= 3)
			pdfCounter = 0;
		
		// Write the table names first into the file.
		for(i = 0; i < num_fields; i++) {
			if (i == 0 && fieldsFound == 0) {
					while(field = mysql_fetch_field(result)) {
						HPDF_Page_ShowText (page, field->name);
						HPDF_Page_ShowText (page, " ");
					}
					HPDF_Page_MoveTextPos (page, 0, -20);
					fieldsFound = 1; // -> used for pdf spacing output, so extra white space isn't issued when it loops through to check for field names on each row.
			}
		
		// Then write the rest of the data into the file.
		printValue = row[i];
		printValue = g_strconcat("\"", printValue, NULL);
		printValue = g_strconcat(printValue, "\";", NULL);
		
		HPDF_Page_ShowText (page, row[i]);
		
		if(pdfCounter == 3) {
			HPDF_Page_MoveTextPos (page, 0, -20);
		} else {
			HPDF_Page_ShowText (page, " ");
		}
		pdfCounter++;
      }
      
      // Free up the memory created by the printValue string.
      g_free(printValue);
	}
	
	HPDF_Page_EndText (page);

	//free up any memory used by the record set and to close the connection.
	mysql_free_result(result);
	mysql_close(connection);	

	// Free up the memory created by the query_string.
	g_free(query_string);
	
	// Output query status to the output box in the software.
	printMessage(g_strconcat(fileName, " saved successfully.", NULL));

    // save the document to a file
    HPDF_SaveToFile (pdf, fileName);

    // clean up 
    HPDF_Free (pdf);	
	
	return 0;
}
*/

//Exporting out the database into a csv file.
static int exportTablesSales(char *fileName, gchar *exportQueryString) {
	
	int num_fields;
	int i;
	gchar *query_string;
	gchar *printValue = NULL;
	
	MYSQL_FIELD *field;

	/* Connection Error */
	if(connectToServer() == 1)
		return 1;
	
	if(exportQueryString != NULL)	
		query_string = g_strconcat(exportQueryString, NULL);
	else
		query_string = g_strconcat("SELECT id, partNo, partDesc, soldTo, invoiceNo, cost, price, dateSold FROM ", SOLD_TABLES, NULL);

	mysql_query(connection, query_string);
	
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);
		
	FILE *file;
	
	// Open the file for writing. Overwrite file if it exists.
	file = fopen(fileName, "w"); 

	// Export data into a file.
	while ( ( row = mysql_fetch_row(result)) ) {
		/* 
			Special formatting for fprint and printf.
			\n is for a new line
			\" is for printing a quotation
		*/
		
		// Write the table names first into the file.
		for(i = 0; i < num_fields; i++) {
			if (i == 0) {
				int fieldCounter = 0;
					while(field = mysql_fetch_field(result)) {
						if(fieldCounter == 0)
							fprintf(file, "%s", "\"");
						else
							fprintf(file, ";%s", "\"");
							
						fprintf(file, "%s", field->name);
						fprintf(file, "%s", "\"");

						fieldCounter++;
					}
				fprintf(file, "%s", "\n");
			}
		
			// Then write the rest of the data into the file.
			if(i == 0)
				printValue = g_strconcat("\"", row[i], "\"", NULL);
			else
				printValue = g_strconcat(";\"", row[i], "\"", NULL);
		
		fprintf(file, "%s", row[i] ? printValue : "\n");
		g_free(printValue);
      }
	}
	
	// Close the file.
	fclose(file); 	

	//free up any memory used by the record set and to close the connection.
	mysql_free_result(result);
	mysql_close(connection);	

	// Free up the memory created by the query_string.
	g_free(query_string);
	
	// Output query status to the output box in the software.
	//printMessage(g_strconcat(fileName, " saved successfully.", NULL));
	
	return 0;

/* fopen reference *********
txt binary  description
r 	rb 		open for reading 	beginning
w 	wb 		open for writing (creates file if it doesn't exist). Deletes content and overwrites the file. 	beginning
a 	ab 		open for appending (creates file if it doesn't exist) 	end
r+ 	rb+ 	r+b 	open for reading and writing 	beginning
w+ 	wb+ 	w+b 	open for reading and writing. Deletes content and overwrites the file. 	beginning
a+ 	ab+ 	a+b 	open for reading and writing (append if file exists) 	end
*/ 	
}

//Exporting out the database into a csv file.
static int exportTables(char *fileName, gchar *exportQueryString) {
	
	int num_fields;
	int i;
	gchar *query_string;
	gchar *printValue = NULL;
	
	MYSQL_FIELD *field;

	/* Connection Error */
	if(connectToServer() == 1)
		return 1;
	
	//query_string = "SELECT barcode, name, description, price, stock from scan";
	//query_string = g_strconcat("SELECT id, partNo, description, replacePart, cost, costAvg, price, stock, lowStockLvl, lastSold FROM ", mysqlTables, NULL);
	if(exportQueryString != NULL)	
		query_string = g_strconcat(exportQueryString, NULL);
	else
		query_string = g_strconcat("SELECT id, partNo, description, manufacturer, replacePart, cost, costAvg, price, stock, lowStockLvl, lastSold, totalSold FROM ", mysqlTables, " WHERE stock !='' AND stock !='0'", NULL);

	mysql_query(connection, query_string);
	
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);
		
	FILE *file;
	
	// Open the file for writing. Overwrite file if it exists.
	file = fopen(fileName, "w"); 

	// Export data into a file.
	while ( ( row = mysql_fetch_row(result)) ) {
		/* 
			Special formatting for fprint and printf.
			\n is for a new line
			\" is for printing a quotation
		*/
		
		// Write the table names first into the file.
		for(i = 0; i < num_fields; i++) {
			if (i == 0) {
				int fieldCounter = 0;
					while(field = mysql_fetch_field(result)) {
						if(fieldCounter == 0)
							fprintf(file, "%s", "^"); //fprintf(file, "%s", "\"");
						else
							fprintf(file, ";%s", "^"); //fprintf(file, ";%s", "\"");
							
						fprintf(file, "%s", field->name);
						fprintf(file, "%s", "^"); //fprintf(file, "%s", "\"");

						fieldCounter++;
					}
				fprintf(file, "%s", "\n");
			}
		
			// Then write the rest of the data into the file.
			if(i == 0)
				printValue = g_strconcat("^", row[i], "^", NULL); //printValue = g_strconcat("\"", row[i], "\"", NULL);
			else
				printValue = g_strconcat(";^", row[i], "^", NULL); //printValue = g_strconcat(";\"", row[i], "\"", NULL);

		fprintf(file, "%s", row[i] ? printValue : "\n");
		g_free(printValue);		
      }
	}
	
	// Close the file.
	fclose(file); 	

	//free up any memory used by the record set and to close the connection.
	mysql_free_result(result);
	mysql_close(connection);	

	// Free up the memory created by the query_string.
	g_free(query_string);
	
	// Output query status to the output box in the software.
	//printMessage(g_strconcat(fileName, " saved successfully.", NULL));
	
	return 0;

/* fopen reference *********
txt binary  description
r 	rb 		open for reading 	beginning
w 	wb 		open for writing (creates file if it doesn't exist). Deletes content and overwrites the file. 	beginning
a 	ab 		open for appending (creates file if it doesn't exist) 	end
r+ 	rb+ 	r+b 	open for reading and writing 	beginning
w+ 	wb+ 	w+b 	open for reading and writing. Deletes content and overwrites the file. 	beginning
a+ 	ab+ 	a+b 	open for reading and writing (append if file exists) 	end
*/ 	
}

//Exporting out the database into a csv file.
static int exportCASL(char *fileName, gchar *exportQueryString) {
	
	int num_fields;
	int i;
	int query_state;
	gchar *query_string;
	gchar *printValue = NULL;
	
	MYSQL_FIELD *field;

	/* Connection Error */
	if(connectToServer() == 1)
		return 1;
		
    // Select the database.
    query_string = g_strdup(CASL_DATABASE);	
	query_state = mysql_select_db(connection, query_string);	
	g_free(query_string);
	
	if(exportQueryString != NULL)	
		query_string = g_strdup(exportQueryString);
	else
		query_string = g_strconcat("SELECT id, emailAddress, ipAddress, FROM_UNIXTIME(dateConfirmed), confirmed, emailSent, category, company, position, phoneNumber FROM ", CASL_TABLES, " WHERE stock !='' AND stock !='0'", NULL);

	mysql_query(connection, query_string);
	
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);
		
	FILE *file;
	
	// Open the file for writing. Overwrite file if it exists.
	file = fopen(fileName, "w"); 

	// Export data into a file.
	while ( ( row = mysql_fetch_row(result)) ) {
		/* 
			Special formatting for fprint and printf.
			\n is for a new line
			\" is for printing a quotation
		*/
		
		// Write the table names first into the file.
		for(i = 0; i < num_fields; i++) {
			if (i == 0) {
				int fieldCounter = 0;
					while(field = mysql_fetch_field(result)) {
						if(fieldCounter == 0)
							fprintf(file, "%s", "^"); //fprintf(file, "%s", "\"");
						else
							fprintf(file, ";%s", "^"); //fprintf(file, ";%s", "\"");
							
						fprintf(file, "%s", field->name);
						fprintf(file, "%s", "^"); //fprintf(file, "%s", "\"");

						fieldCounter++;
					}
				fprintf(file, "%s", "\n");
			}
		
			// Then write the rest of the data into the file.
			if(i == 0)
				printValue = g_strconcat("^", row[i], "^", NULL); //printValue = g_strconcat("\"", row[i], "\"", NULL);
			else
				printValue = g_strconcat(";^", row[i], "^", NULL); //printValue = g_strconcat(";\"", row[i], "\"", NULL);

		fprintf(file, "%s", row[i] ? printValue : "\n");
		g_free(printValue);
      }
	}
	
	// Close the file.
	fclose(file); 	

	//free up any memory used by the record set and to close the connection.
	mysql_free_result(result);
	mysql_close(connection);	

	// Free up the memory created by the query_string.
	g_free(query_string);
	
	// Output query status to the output box in the software.
	//printMessage(g_strconcat(fileName, " saved successfully.", NULL));
	
	return 0;

/* fopen reference *********
txt binary  description
r 	rb 		open for reading 	beginning
w 	wb 		open for writing (creates file if it doesn't exist). Deletes content and overwrites the file. 	beginning
a 	ab 		open for appending (creates file if it doesn't exist) 	end
r+ 	rb+ 	r+b 	open for reading and writing 	beginning
w+ 	wb+ 	w+b 	open for reading and writing. Deletes content and overwrites the file. 	beginning
a+ 	ab+ 	a+b 	open for reading and writing (append if file exists) 	end
*/ 	

/*
BEGIN:VCARD
VERSION:3.0
N:Gump;Forrest;Mr.
FN:Forrest Gump
ORG:Bubba Gump Shrimp Co.
TITLE:Shrimp Man
PHOTO;VALUE=URL;TYPE=GIF:http://www.example.com/dir_photos/my_photo.gif
TEL;TYPE=WORK,VOICE:(111) 555-12121
TEL;TYPE=HOME,VOICE:(404) 555-1212
ADR;TYPE=WORK:;;100 Waters Edge;Baytown;LA;30314;United States of America
LABEL;TYPE=WORK:100 Waters Edge\nBaytown, LA 30314\nUnited States of America
ADR;TYPE=HOME:;;42 Plantation St.;Baytown;LA;30314;United States of America
LABEL;TYPE=HOME:42 Plantation St.\nBaytown, LA 30314\nUnited States of America
EMAIL;TYPE=PREF,INTERNET:forrestgump@example.com
REV:2008-04-24T19:52:43Z
END:VCARD
*/ 
}

int mysqlExportDatabase(GtkWidget *widget, gpointer parentWindow, gchar *exportQueryString) {
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new ("Save File",
				      parentWindow,
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "Untitled document");

    gtk_filter_add(dialog, ".CSV (Comma Separated Value File)", "*.csv");
    gtk_filter_add(dialog, ".TXT (Plain Text File)", "*.txt");
    

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		exportTables(filename, exportQueryString);
		g_free (filename);
	}

	gtk_widget_destroy (dialog);
	
	return 0;
}

int mysqlExportSales(GtkWidget *widget, gpointer parentWindow, gchar *exportQueryString) {
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new ("Save File",
				      parentWindow,
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "Untitled document");

    gtk_filter_add(dialog, ".CSV (Comma Separated Value File)", "*.csv");
    gtk_filter_add(dialog, ".TXT (Plain Text File)", "*.txt"); 
    

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		exportTablesSales(filename, exportQueryString);
		g_free (filename);
	}

	gtk_widget_destroy (dialog);
	
	return 0;
}

int mysqlExportCASL(GtkWidget *widget, gpointer parentWindow, gchar *exportQueryString) {
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new ("Save File",
				      parentWindow,
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "Untitled document");

    gtk_filter_add(dialog, ".CSV (Comma Separated Value File)", "*.csv");
    gtk_filter_add(dialog, ".TXT (Plain Text File)", "*.txt"); 
    

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		exportCASL(filename, exportQueryString);
		g_free (filename);
	}

	gtk_widget_destroy (dialog);
	
	return 0;
}

//Import a csv file data into the database or a new database.
static void importTables(char *fileName) {
	
	g_print("Importing Database");
	
}

static void mysqlImportDatabase(GtkWidget *widget, gpointer parentWindow) {

	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new ("Open File",
				      parentWindow,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		importTables(filename);
		g_free (filename);
	}

	gtk_widget_destroy (dialog);
}
