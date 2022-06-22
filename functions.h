//      functions.h
//      Copyright 2010 Michael Rajotte <michael@michaelrajotte.com>
// 		Common functions for the software.

static void hideGtkWidget(GtkWidget *, gpointer);
static void destroyWidget(GtkWidget *, gpointer);
void showErrorDialog(gchar *, gpointer);
void showMessageDialog(gchar *, gpointer);
int printMessage (gchar *, gpointer);
int connectToServer ();
int checkNetworkConnection ();
int getSpinButtonValue(GtkSpinButton *);
int checkInput(gchar *);
int compareGDouble(gdouble, gdouble, gdouble);
gboolean intrackQueryResult(gchar *, gchar *, MYSQL_RES *, GtkWidget *);
void processDate(GDate *, GtkWidget *);
int database_query(gchar *, gchar *);

// Updates a item in the inventory database.
int database_query(gchar *database, gchar *query) {
	MYSQL *queryConnection, queryMysql;

	mysql_init(&queryMysql);
	queryConnection = mysql_real_connect(&queryMysql,mysqlServer,mysqlUsername,mysqlPassword,MYSQL_DEFAULT_DATABASE, 0, 0, 0);
		
	int query_state;	
	
	// Select the database.
	query_state = mysql_select_db(queryConnection, database);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(queryConnection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(queryConnection));
		printMessage(errorMessage, NULL);
		g_free(errorMessage);		
		
		mysql_close(queryConnection);
		
		return 1;
	}
	
	query_state=mysql_query(queryConnection, query);
	
	// Failed to update the data
	if (query_state !=0) {
		printf(mysql_error(queryConnection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(queryConnection));
		printMessage(errorMessage, NULL);
		g_free(errorMessage);	
				
		mysql_close(queryConnection);

		return 1;
	}
	
	mysql_close(queryConnection);			
	return 0;	
}

// Converts a calendar entry to a gdate
void processDate(GDate *date, GtkWidget *dateEntry) {
	//date = g_date_new();
	g_date_set_parse(date, gtk_entry_get_text(GTK_ENTRY(dateEntry)));
}

// Query the database and return a result.
gboolean intrackQueryResult(gchar *database, gchar *query, MYSQL_RES *intrackResult, GtkWidget *window) {
	
	MYSQL *connect, mysqlConnect;
	MYSQL_ROW row;
		
	mysql_init(&mysqlConnect);

	connect = mysql_real_connect(&mysqlConnect,mysqlServer,mysqlUsername,mysqlPassword,mysqlDatabase,0,NULL,0);	

	if(connect == NULL) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connect));
		printMessage(errorMessage, window);
		g_free(errorMessage);
		
		return FALSE;
	}		
	
	int query_state;	
	
	// Select the database.
	query_state = mysql_select_db(connect, database);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connect), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connect));
		printMessage(errorMessage, window);
		g_free(errorMessage);		
		
		mysql_close(connect);
		
		return FALSE;
	}
	
	query_state=mysql_query(connect, query);
	intrackResult = mysql_store_result(connect);
	
	/* Failed to query and get result */
	if (query_state !=0) {
		printf(mysql_error(connect), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connect));
		printMessage(errorMessage, window);
		g_free(errorMessage);	
				
		mysql_close(connect);

		return FALSE;
	}
	
	
	//mysql_close(connect);
	
	return TRUE;		
}

/* Used to check if a pair of floats or doubles are equal to each other. */
/* Compare if the gdouble f1 is equal with f2 and returns 1 if true and 0 if false */
int compareGDouble(gdouble gd1, gdouble gd2, gdouble precision) {
	//gdouble precision = 0.00001;
	if(((gd1 - precision) < gd2) && ((gd1 + precision) > gd2))
		return 1;
	else
		return 0;
}

/* Destroy a GtkWidget */
static void destroyGtkWidget(GtkWidget *widget, gpointer data) {
	gtk_widget_destroy(GTK_WIDGET(data));
}

static void destroyWidget(GtkWidget *widget, gpointer data) {
	gtk_widget_destroy(GTK_WIDGET(data));
}

static void hideGtkWidget(GtkWidget *widget, gpointer data) {
	gtk_widget_hide_all(GTK_WIDGET(data));
}

int checkInput(gchar *input) {

	if(g_strrstr(input, "'")) {
		printMessage("ERROR: ' not allowed.", NULL);
		return 1;
	}
	
	if(g_strrstr(input, "`")) {
		printMessage("ERROR: ` not allowed.", NULL);
		return 1;
	}
	
	return 0;
}

// Returns back the value of a GtkSpinButton as a int
int getSpinButtonValue(GtkSpinButton *a_spinner) {
   return gtk_spin_button_get_value_as_int (a_spinner);
}

int connectToServer() {
	
	mysql_init(&mysql);

	connection = mysql_real_connect(&mysql,mysqlServer,mysqlUsername,mysqlPassword,mysqlDatabase,0,NULL,0);	
	//connection = mysql_real_connect(&mysql, mysqlServer, mysqlUsername, mysqlPassword, MYSQL_DEFAULT_DATABASE, 0, 0, 0);

	if (connection == NULL) {
		printf(mysql_error(connection), "%d\n");
		
		if(checkNetworkConnection() == 1) {
			return 1;
		} 
		else {
			printMessage(ERROR_CONNECTION, NULL);
			return 1;
		}
	}	
	
	return 0;
}

int thread_checkNetworkConnection() {
	
	// Connect to the server
	mysql_init(&mysql);	
	connection = mysql_real_connect(&mysql, mysqlServer, mysqlUsername, mysqlPassword, MYSQL_DEFAULT_DATABASE, 0, 0, 0);

	// If connection failed
	if (connection == NULL) {
		printMessage(ERROR_NETWORK, NULL);

		mysql_close(connection);
		//gtk_widget_set_sensitive(noteBookSetSettings, FALSE);
		return 1;
	}
	else {
		//gtk_widget_set_sensitive(noteBookSetSettings, TRUE);
	}	
	
	mysql_close(connection);
	return 0;
}	

int checkNetworkConnection() {
	
	
   // g_thread_create((GThreadFunc)thread_checkNetworkConnection, NULL, FALSE, NULL);

	//return 0;
	
	
	// Connect to the server
	mysql_init(&mysql);	
	connection = mysql_real_connect(&mysql, mysqlServer, mysqlUsername, mysqlPassword, MYSQL_DEFAULT_DATABASE, 0, 0, 0);

	// If connection failed
	if (connection == NULL) {
		printMessage(ERROR_NETWORK, NULL);
		printf(mysql_error(connection), "%d\n");

		mysql_close(connection);
		//gtk_widget_set_sensitive(noteBookSetSettings, FALSE);

		return 1;
	}
	else {
		//gtk_widget_set_sensitive(noteBookSetSettings, TRUE);
	}
	
	mysql_close(connection);

	return 0;
}

// Prints messages to the output message window.
int printMessage(gchar *message, gpointer windowWidget) {
	g_print("%s\n", message);
	
	if(windowWidget != NULL)
		showErrorDialog(message, windowWidget);
	//else
		//showErrorDialog(message, window);
			
	/*
	// Text buffer window stuff.
	gchar *str = NULL;
	GtkTextBuffer           *buffer;
	GtkTextIter             start, end;
	gint	lineCount;
	
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
	gtk_text_buffer_get_bounds (buffer, &start, &end);

	str = g_strconcat("--> ", message, "\n", NULL);
	gtk_text_buffer_insert(buffer, &start, str, -1);

	if(windowWidget != NULL)
		showErrorDialog(message, windowWidget);
	else
		showErrorDialog(message, window);
	
	g_free(str);

	lineCount = gtk_text_buffer_get_line_count(buffer);
	
	// Check how many lines in the text buffer, and start removing when the limit has been reached
	if(lineCount > TEXT_BUFFER_LINES) {
		gtk_text_buffer_get_bounds (buffer, &start, &end);
		lineCount = TEXT_BUFFER_LINES - 1;
		gtk_text_buffer_get_iter_at_line(buffer, &start, lineCount);
		gtk_text_buffer_delete(buffer, &start, &end);
	}
	
	return 0;
	*/
}

void showErrorDialog(gchar *messageText, gpointer windowWidget) {
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(GTK_WINDOW(windowWidget),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "%s", messageText);
  gtk_window_set_title(GTK_WINDOW(dialog), "Error");
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

void showMessageDialog(gchar *messageText, gpointer windowWidget)
{

	if(windowWidget != NULL) {
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new(GTK_WINDOW(windowWidget),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "%s", messageText);
	gtk_window_set_title(GTK_WINDOW(dialog), "Information");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	}
	else {
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "%s", messageText);
	gtk_window_set_title(GTK_WINDOW(dialog), "Information");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);		
		
	}
}



static gchar getYear() {
	
    struct tm *ptr;
    time_t lt;
    char str[80];
	char *text;
	
    lt = time(NULL);
    ptr = localtime(&lt);

    strftime(str, 100, "%G", ptr);
	
	//theYear = g_strdup(str);
	
	//theYear = strdup(text);
	
	//g_print(str);
	//return theYear;
}

static gboolean noteBookPageTimer() {

	//noteBookPage=gtk_notebook_get_current_page(mainNotebook);

  return TRUE;
} 


// Set focus to the scan entry widget
void focusScanWindow(GtkObject *widget, gpointer entryWidget) {

	gtk_widget_grab_focus(GTK_WIDGET(entryWidget));

}

// Exits the program
void on_window_destroy (GtkObject *object, gpointer user_data) {
    gtk_main_quit ();
}


// List the different file types to save to.
void gtk_filter_add(GtkWidget *file_chooser, const gchar *title, const gchar *pattern)
{
    //gchar **patterns;
   // gint i;

    GtkFileFilter *f = gtk_file_filter_new();
    gtk_file_filter_set_name(f, title);
    
    gtk_file_filter_add_pattern(f, pattern);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), f);
    
	//g_free(f);
    
} 

GdkPixbuf *create_pixelbuffer_icon(const gchar * filename) {
   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   if(!pixbuf) {
      fprintf(stderr, "%s\n", error->message);
      g_error_free(error);
   }

   return pixbuf;
}

// Load a image into memory
GdkPixbuf *create_pixbuf_image(const gchar * filename) {

   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   if(!pixbuf) {
      fprintf(stderr, "%s\n", error->message);
      g_error_free(error);
   }

   return pixbuf;	
	
}

void editItemPrint (GtkWidget *widget, gpointer *data) {
	
	g_print ("editItemPrint");
}

