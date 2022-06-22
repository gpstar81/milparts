//      systray.h
//      Copyright 2010 Michael Rajotte <michael@michaelrajotte.com>
// 		System icon tray

void tray_icon_on_click(GtkStatusIcon *status_icon, gpointer mainWindow) {
		
		if(minimizeToTray == 1) {
			gtk_widget_show_all(mainWindow);
			//gtk_notebook_set_current_page(mainNotebook, lastNoteBookPage); // -> Set the notebook back to the page we were last on.
			//gtk_notebook_set_current_page(inventoryNoteBook, inventoryLastNoteBookPage); // -> Set the notebook back to the page we were last on.
			
			
			minimizeToTray = 0;			
		}
		else if(minimizeToTray == 0) {
			//lastNoteBookPage = noteBookPage; // -> record which page we are in the notebook before minimizing it.
			//inventoryLastNoteBookPage = inventoryNoteBookPage;
			gtk_widget_hide_all(mainWindow);
			minimizeToTray = 1;
		}
}

void tray_icon_on_menu(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data) {
        printf("Popup menu\n");
}


static GtkStatusIcon *create_tray_icon(gpointer mainWindow) {
        
        GtkStatusIcon *tray_icon;

       	tray_icon = gtk_status_icon_new_from_file (PROGRAM_ICON);

        g_signal_connect(G_OBJECT(tray_icon), "activate", G_CALLBACK(tray_icon_on_click), mainWindow);
        
        g_signal_connect(G_OBJECT(tray_icon), "popup-menu", G_CALLBACK(tray_icon_on_menu), NULL);
        
       /*
        gtk_status_icon_set_from_icon_name(tray_icon, 
                                           GTK_STOCK_MEDIA_STOP);
        */
        gtk_status_icon_set_tooltip(tray_icon, PROGRAM_NAME);
        gtk_status_icon_set_visible(tray_icon, TRUE);

        return tray_icon;
}
