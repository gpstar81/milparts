//      milparts.c
//      Copyright 2010 - 2014 Michael Rajotte <michael@michaelrajotte.com>
// 		Parts Inventory Management, Tracking & Account Management.

// Compiler settings:
// gcc $TIMESTAMP -o barcode barcode.c -L/usr/include/mysql -lmysqlclient -I/usr/include/mysql $(pkg-config --cflags --libs gtk+-2.0 gmodule-2.0)
// gcc $TIMESTAMP -o barcode barcode.c -lm -I/usr/local/include barcodeInit.c common.c png.c font.c code.c code128.c `libpng12-config --I_opts --L_opts --ldflags` -lz -L/usr/include/mysql -lmysqlclient -I/usr/include/mysql $(pkg-config --cflags --libs gtk+-2.0 gmodule-2.0)
// Packages libgtk2.0-dev & libmysql15-dev (gtk+2.14 or higher is required)

// switch mysqlclient to libdrizzle for a lgpl license.
// *** MEMORY LEAKS: GtkTreePath *path; <- search for that term and make sure to put a free_path(path). All the key press selections in intrack need to be fixed. ***

const char *DateTimeCompile = __TIMESTAMP__; // --> Setting up the build date, recieves value from the compile time flags when building the program.
//const char *DateBaseFile = __BASE_FILE__;

// #include <winsock.h> // -> required for mysql.h in mingw32 windows builds
#include <mysql.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <time.h>

// #include "hpdf.h" // haru pdf export library
#include "milparts.h"
#include "settings.h"
#include "messages.h"
//#include "printConfig.h"

#include "functions.h"
#include "configuration.h"
#include "systray.h"
//#include "users.h"
//#include "createbarcode.h"
//#include "databaseExport.h"
//#include "checkout.h"
//#include "print.h"
//#include "reports.h"

// Use lock tables to prevent multiple sessions from working on the database, and transactions to keep data reliable.
// http://www.mysqltutorial.org/mysql-transaction.aspx

int main( int argc, char **argv ) {

    // Initialize GTK+
    gtk_init(&argc, &argv);
    
	if(! g_thread_supported())
       gdk_threads_init(); //g_thread_init( NULL );

    //g_thread_init (NULL);
	//gdk_threads_init();

	// Load the program.
	loadMainWindow(); // Comment out loadConfigurationSettings() in loadMainWindow()
	
    // Start main loop
    gtk_main();
    
	return 0;
}

void show_helpfile( GtkWidget *menuitem ) {
	GError *error = NULL;
	char *uri;
	
	uri = "ghelp:/home/michael/Builds/c/intrack/manual/manual.xml";
	
	gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &error);
	
	if (error) {
		printMessage("ERROR: Could not open the help file.", window);
		//showErrorDialog("ERROR: Could not open the help file.", window);
		g_error_free (error);
 	}
}

void show_about( GtkWidget *widget, gpointer data ) {
	// Getting the build date
	gchar *dateBuilt;
	dateBuilt = g_strconcat(PROGRAM_COPYRIGHT, " - Build Date: ", DateTimeCompile, NULL);
	
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(PROGRAM_ICON, NULL);
	GtkWidget *dialog = gtk_about_dialog_new();
	
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), PROGRAM_NAME);
	gtk_window_set_icon(GTK_WINDOW(dialog), create_pixelbuffer_icon(PROGRAM_ICON));
	
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), PROGRAM_VERSION);
	
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), dateBuilt);
	
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);
	
	g_free(dateBuilt);

	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), PROGRAM_DESCRIPTION);
	//gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), PROGRAM_WEBSITE);
	
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pixbuf);
	g_object_unref(pixbuf), pixbuf = NULL;
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

int loadMainWindow () {
	GtkBuilder *builder;
    GError     *error = NULL;
    
    GtkBuilder *builder2;
    GError		*error2 = NULL;
    
    GtkBuilder *builder3;
    GError		*error3 = NULL;    
    
    GtkBuilder *builder4;
    GError		*error4 = NULL;     
    
    GtkBuilder *builder5;
    GError		*error5 = NULL;       
    
    GtkBuilder *builder6;
    GError		*error6 = NULL;       
    
    GtkBuilder *builder7;
    GError		*error7 = NULL;         
    
    GtkWidget 		*about;
    GtkWidget 		*helpContentsButton;
    GtkWidget		*quitProgram;
    GtkWidget		*button_settingsMenuButton;
    GtkWidget		*configButton;
    
    GtkWidget		*statusBar;

	//GtkStatusIcon *tray_icon;
	GdkPixbuf *scanner;

    // Create new GtkBuilder object
    builder = gtk_builder_new();
    if(!gtk_builder_add_from_file(builder,GLADE_FILE,&error)) {
        g_warning("%s", error->message);
        g_free(error);
        return(1);
    }
  
	builder2 = gtk_builder_new();
    if(!gtk_builder_add_from_file(builder2,PARTSALES_FILE,&error2)) {
        g_warning("%s", error2->message);
        g_free(error2);
        return(1);
    }	
    
	builder3 = gtk_builder_new();
    if(!gtk_builder_add_from_file(builder3,ACCOUNTS_EDITOR_FILE,&error3)) {
        g_warning("%s", error3->message);
        g_free(error3);
        return(1);
    }	 
    
    builder4 = gtk_builder_new();
    if(!gtk_builder_add_from_file(builder4,WARRANTY_FILE,&error4)) {
        g_warning("%s", error4->message);
        g_free(error4);
        return(1);
    }	  
    
    builder5 = gtk_builder_new();
    if(!gtk_builder_add_from_file(builder5,SALES_FILE,&error5)) {
        g_warning("%s", error5->message);
        g_free(error5);
        return(1);
    }	
    
    builder6 = gtk_builder_new();
    if(!gtk_builder_add_from_file(builder6,SYSTEM_FILE,&error6)) {
        g_warning("%s", error6->message);
        g_free(error6);
        return(1);
    }
    
    builder7 = gtk_builder_new();
    if(!gtk_builder_add_from_file(builder7,CASL_FILE,&error7)) {
        g_warning("%s", error7->message);
        g_free(error7);
        return(1);
    }    
 
    // Get main window pointer from UI.
    window = GTK_WIDGET(gtk_builder_get_object(builder, "mainWindow"));
    gtk_window_set_title (GTK_WINDOW(window), PROGRAM_TITLE);
	
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(PROGRAM_ICON, NULL);
    gtk_window_set_icon(GTK_WINDOW(window), create_pixelbuffer_icon(PROGRAM_ICON));
	g_object_unref(pixbuf), pixbuf = NULL;

	statusBar = GTK_WIDGET(gtk_builder_get_object(builder, "statusBar"));

    // Exit program button from file / quit.
    quitProgram = GTK_WIDGET(gtk_builder_get_object(builder, "ExitProgram"));
   	g_signal_connect(G_OBJECT(quitProgram), "activate", G_CALLBACK(on_window_destroy), window);

    // Load the configuration file so we can start connecting to the database. <-- configuration.h
	loadConfigurationSettings(window);
		
	// Intro Message ===================================================
	GtkWidget *introMessage = GTK_WIDGET(gtk_builder_get_object(builder, "introLabel"));
	gchar *introText = g_strconcat("Inventory, Sales and Accounts Management",  NULL);
	gtk_label_set_text(GTK_LABEL(introMessage), introText);
	g_free(introText);
	// Intro Message ===================================================

   	//INVENTORY=========================================================
    // load up the inventory <-- inventory.c
    GtkWidget	*inventoryButton;
    GtkWidget	*inventoryWindow;
    
    inventoryWindow = GTK_WIDGET(gtk_builder_get_object(builder, "inventoryWindow"));
    gtk_window_set_title(GTK_WINDOW(inventoryWindow), "Miller Canada Inventory");
    gtk_window_set_position(GTK_WINDOW(inventoryWindow), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(inventoryWindow), GTK_WINDOW(window));
    
    inventoryButton = GTK_WIDGET(gtk_builder_get_object(builder, "inventoryButton"));
    g_signal_connect(G_OBJECT(inventoryButton), "clicked", G_CALLBACK(showWindow), inventoryWindow);
    g_signal_connect(G_OBJECT(inventoryWindow), "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), inventoryWindow);
    
    initalizeInventory(builder, mysqlDatabase, mysqlTables, inventoryWindow);
   	//INVENTORY=========================================================
   	
   	//PART SALES========================================================
    // load up the part sales <-- partSales.c
    GtkWidget	*partSalesButton;
    GtkWidget	*partSalesWindow;

    partSalesWindow = GTK_WIDGET(gtk_builder_get_object(builder2, "partSalesWindow"));
    gtk_window_set_title(GTK_WINDOW(partSalesWindow), "Miller Canada Sales");
    gtk_window_set_position(GTK_WINDOW(partSalesWindow), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(partSalesWindow), GTK_WINDOW(window));
    
    partSalesButton = GTK_WIDGET(gtk_builder_get_object(builder, "partSalesButton"));
    g_signal_connect(G_OBJECT(partSalesButton), "clicked", G_CALLBACK(showWindow), partSalesWindow);
    g_signal_connect(G_OBJECT(partSalesWindow), "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), partSalesWindow);

    initalizePartSales(builder2, mysqlDatabase, mysqlTables, partSalesWindow);
   	//PART SALES========================================================
   	
   	//ACCOUNTS =========================================================
    // load up the accounts <-- accounts.c
    GtkWidget	*accountsButton;
    GtkWidget	*accountsWindow;

    accountsWindow = GTK_WIDGET(gtk_builder_get_object(builder3, "accountsWindow"));
    gtk_window_set_title(GTK_WINDOW(accountsWindow), "Miller Canada Accounts");
    gtk_window_set_position(GTK_WINDOW(accountsWindow), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(accountsWindow), GTK_WINDOW(window));

    accountsButton = GTK_WIDGET(gtk_builder_get_object(builder, "accountsButton"));
    g_signal_connect(G_OBJECT(accountsButton), "clicked", G_CALLBACK(showWindow), accountsWindow);
    g_signal_connect(G_OBJECT(accountsWindow), "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), accountsWindow);

    loadAccounts(builder3, accountsWindow);
   	//ACCOUNTS =========================================================

   	//CASL =============================================================
    GtkWidget	*caslButton;
    GtkWidget	*caslWindow;

    caslWindow = GTK_WIDGET(gtk_builder_get_object(builder7, "caslWindow"));
    gtk_window_set_title(GTK_WINDOW(caslWindow), "Miller Canada CASL Emails");
    gtk_window_set_position(GTK_WINDOW(caslWindow), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(caslWindow), GTK_WINDOW(window));

    caslButton = GTK_WIDGET(gtk_builder_get_object(builder, "caslButton"));
    g_signal_connect(G_OBJECT(caslButton), "clicked", G_CALLBACK(showWindow), caslWindow);
    g_signal_connect(G_OBJECT(caslWindow), "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), caslWindow);

    loadCASL(builder7, caslWindow);
   	//CASL =============================================================
   	
   	//WARRANTY =========================================================
    GtkWidget	*warrantyButton;
    GtkWidget	*warrantyWindow;

    warrantyWindow = GTK_WIDGET(gtk_builder_get_object(builder4, "warrantyWindow"));
    gtk_window_set_title(GTK_WINDOW(warrantyWindow), "Miller Canada Warranties");
    gtk_window_set_position(GTK_WINDOW(warrantyWindow), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(warrantyWindow), GTK_WINDOW(window));

    warrantyButton = GTK_WIDGET(gtk_builder_get_object(builder, "warrantyButton"));
    g_signal_connect(G_OBJECT(warrantyButton), "clicked", G_CALLBACK(showWindow), warrantyWindow);
    g_signal_connect(G_OBJECT(warrantyWindow), "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), warrantyWindow);

    loadWarranty(builder4, warrantyWindow);
   	//WARRANTY ========================================================= 
   	
   	//SYSTEMS ==========================================================
    GtkWidget	*systemButton;
    GtkWidget	*systemWindow;

    systemWindow = GTK_WIDGET(gtk_builder_get_object(builder6, "systemWindow"));
    gtk_window_set_title(GTK_WINDOW(systemWindow), "Miller Canada Systems Inventory");
    gtk_window_set_position(GTK_WINDOW(systemWindow), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(systemWindow), GTK_WINDOW(window));

    systemButton = GTK_WIDGET(gtk_builder_get_object(builder, "systemButton"));
    g_signal_connect(G_OBJECT(systemButton), "clicked", G_CALLBACK(showWindow), systemWindow);
    g_signal_connect(G_OBJECT(systemWindow), "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), systemWindow);

    loadSystems(builder6, systemWindow);
   	//SYSTEMS ==========================================================    	
   	
   	//SALES ============================================================
    GtkWidget	*salesButton;
    GtkWidget	*salesWindow;

    salesWindow = GTK_WIDGET(gtk_builder_get_object(builder5, "saleWindow"));
    gtk_window_set_title(GTK_WINDOW(salesWindow), "Miller Canada Sales");
    gtk_window_set_position(GTK_WINDOW(salesWindow), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(salesWindow), GTK_WINDOW(window));

    salesButton = GTK_WIDGET(gtk_builder_get_object(builder, "ordersButton"));
    g_signal_connect(G_OBJECT(salesButton), "clicked", G_CALLBACK(showWindow), salesWindow);
    g_signal_connect(G_OBJECT(salesWindow), "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), salesWindow);

    loadSales(builder5, salesWindow);
   	//SALES ============================================================       	    	
   	  	
    button_settingsMenuButton = GTK_WIDGET (gtk_builder_get_object (builder, "settingsMenuButton"));
    configButton = GTK_WIDGET(gtk_builder_get_object(builder, "configButton"));

	// About widget box
	scanner = gtk_image_get_pixbuf(GTK_IMAGE(gtk_image_new_from_file(PROGRAM_ICON)));
	about = GTK_WIDGET (gtk_builder_get_object (builder, "aboutButton"));
	g_signal_connect(G_OBJECT(about), "activate", G_CALLBACK(show_about), (gpointer) window);
	
    // Connect signals
    gtk_builder_connect_signals(builder, NULL );
    gtk_builder_connect_signals(builder2, NULL );
    gtk_builder_connect_signals(builder3, NULL );
    gtk_builder_connect_signals(builder4, NULL );
    gtk_builder_connect_signals(builder5, NULL );
    gtk_builder_connect_signals(builder6, NULL );
    gtk_builder_connect_signals(builder7, NULL );
 
    // Destroy builder, since we don't need it anymore
    g_object_unref(G_OBJECT(builder));
    g_object_unref(G_OBJECT(builder2));
    g_object_unref(G_OBJECT(builder3));
    g_object_unref(G_OBJECT(builder4));
    g_object_unref(G_OBJECT(builder5));
	g_object_unref(G_OBJECT(builder6));
	g_object_unref(G_OBJECT(builder7));
	
	// Setting Configuration Menu
    g_signal_connect(G_OBJECT(button_settingsMenuButton), "activate", G_CALLBACK(configureSettings), window);
    g_signal_connect(G_OBJECT(configButton), "clicked", G_CALLBACK(configureSettings), window);
    
	// Display the System Tray Icon (Linux & UNIX systems only)
	tray_icon = create_tray_icon(window);

    // Show window. All other widgets are automatically shown by GtkBuilder
    gtk_widget_show_all( window );
        	
	return 0;
}

static void showWindow(GtkWidget *widget, GtkWidget *window) {
	//gtk_window_present(GTK_WINDOW(window));
	gtk_widget_show_all(window);
}
