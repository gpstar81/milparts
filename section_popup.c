//      section_popup.c
//      Copyright 2014 Michael Rajotte <michael@michaelrajotte.com>
// 		For changing and setting which category a item belongs to.

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
#include "section_popup.h"


void sectionPopup(GtkWidget *parentWindow, gchar *itemCode, GtkWidget *parentTree) {
	
	GtkBuilder *builder;
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, CATEGORY_POPUP_FILE, NULL);

	IntrackCategories *intrackCat;
	intrackCat = (IntrackCategories*) g_malloc (sizeof (IntrackCategories));
	intrackCat->itemCode = g_strdup(itemCode);
	intrackCat->selectedSection = g_strdup(NULL);
	intrackCat->selectedSectionName = g_strdup(NULL);
	
	intrackCat->parentTree = parentTree; // If need to update a tree selection from another window, then pass it's tree and path info over
	
	// Widget window
	intrackCat->mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "groupWindow"));
	gtk_window_set_title (GTK_WINDOW(intrackCat->mainWindow), "Select a category");
	gtk_window_set_transient_for(GTK_WINDOW(intrackCat->mainWindow), GTK_WINDOW(parentWindow));
	gtk_window_set_modal(GTK_WINDOW(intrackCat->mainWindow), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(intrackCat->mainWindow), FALSE);
	gtk_widget_set_size_request(intrackCat->mainWindow, 400, 250);	
	gtk_window_set_position(GTK_WINDOW(intrackCat->mainWindow), GTK_WIN_POS_CENTER);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(intrackCat->mainWindow), TRUE);
	g_signal_connect(intrackCat->mainWindow, "destroy", G_CALLBACK(catFreeMemory), intrackCat); // Free memory when the window widget is destroyed

    // cancel button
    GtkWidget	*closeButton;
    closeButton = GTK_WIDGET(gtk_builder_get_object(builder, "cancelButton"));
   	g_signal_connect(G_OBJECT(closeButton), "clicked", G_CALLBACK(closeWindow), intrackCat);
   	
   	intrackCat->applyButton = GTK_WIDGET(gtk_builder_get_object(builder, "applyButton"));
   	g_signal_connect(G_OBJECT(intrackCat->applyButton), "clicked", G_CALLBACK(changeSections), intrackCat);
   	gtk_widget_set_sensitive(GTK_WIDGET(intrackCat->applyButton), FALSE);
   	
   	// no category button
   	GtkWidget	*noCatButton;
   	noCatButton = GTK_WIDGET(gtk_builder_get_object(builder, "noCatButton"));
   	g_signal_connect(G_OBJECT(noCatButton), "clicked", G_CALLBACK(noSectionConfirm), intrackCat);
	
	// Setup the category tree
	intrackCat->catContainer = GTK_WIDGET(gtk_builder_get_object(builder, "categoryScroll"));
	intrackCat->catTree = gtk_tree_view_new();

	setupSectionTree(intrackCat); // Setup the tree
	
	getSections(intrackCat);

	gtk_container_add(GTK_CONTAINER(intrackCat->catContainer), intrackCat->catTree);
	g_signal_connect(intrackCat->catTree, "button-press-event", G_CALLBACK(treeButtonPress), intrackCat);
	g_signal_connect(intrackCat->catTree, "key-press-event", G_CALLBACK(treeKeyPress), intrackCat);

    g_object_unref(G_OBJECT(builder));
	gtk_widget_show_all(intrackCat->mainWindow);	
}

static void closeWindow(GtkWidget *widget, IntrackCategories *intrackCat) {
	gtk_widget_destroy(intrackCat->mainWindow);
}

/* Free up all the memory used by the category window and structure */
static void catFreeMemory(GtkWidget *widget, IntrackCategories *intrackCat) {
	g_free(intrackCat->itemCode);
	g_free(intrackCat->selectedSection);
	g_free(intrackCat->selectedSectionName);
	
	if(widget)
		gtk_widget_destroy(widget);
		
	g_free(intrackCat);
}

static void noSectionConfirm(GtkWidget *widget, IntrackCategories *intrackCat) {
	
	GtkWidget *dialog;
	gint widgetDialog;
	gchar *message;
	
	message = g_strdup("Set category to not belong to a section ?");
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(intrackCat->mainWindow), 
			GTK_DIALOG_DESTROY_WITH_PARENT, 
			GTK_MESSAGE_WARNING, 
			GTK_BUTTONS_YES_NO, 
			message);
	
	g_free(message);
			
	gtk_window_set_title(GTK_WINDOW(dialog), "Set to category section?");
	widgetDialog = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	switch (widgetDialog) {
		case GTK_RESPONSE_YES:
			g_free(intrackCat->selectedSection);
			intrackCat->selectedSection = g_strdup(NULL);
			changeSections(NULL, intrackCat);
			break;
		case GTK_RESPONSE_NO:
			break;
	}
}

static void changeSections(GtkWidget *widget, IntrackCategories *intrackCat) {
	if(intrackCat->selectedSection == NULL && widget != NULL)
		printMessage("ERROR: Please select a category.");
	else {
		int update_state;
		gchar *query;
		
		// If we update category parents via inventory_cat.c
		if(intrackCat->selectedSection == NULL)
			query = g_strconcat("UPDATE `", CATEGORY_TABLES, "` SET section='0' WHERE id='", intrackCat->itemCode, "'", NULL);
		else
			query = g_strconcat("UPDATE `", CATEGORY_TABLES, "` SET section='", intrackCat->selectedSection, "' WHERE id='", intrackCat->itemCode, "'", NULL);
			
		update_state = categoryUpdate(query); // -> inventory_cat.c
			
		g_free(query);
		
		// Update a tree from a parent window if required
		if(update_state == 0 && intrackCat->parentTree != NULL)
			updateParentTree(intrackCat);
			
		closeWindow(NULL, intrackCat);
	}
}

static void updateParentTree(IntrackCategories *intrackCat) {
    GtkTreeModel 		*model;
    GtkTreePath			*path;
    GtkTreeSelection	*selection;
	GtkTreeRowReference *ref;
	GList *rows, *ptr, *references = NULL;    
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(intrackCat->parentTree));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->parentTree));
	rows = gtk_tree_selection_get_selected_rows(selection, &model);
	
	ptr = rows;
	
	// Create tree row references to all of the selected rows. In this case, there should only be 1 row selected in the parent tree.
	while(ptr != NULL) {
		ref = gtk_tree_row_reference_new(model, (GtkTreePath*) ptr->data);
		references = g_list_prepend(references, gtk_tree_row_reference_copy(ref));
		gtk_tree_row_reference_free(ref);
		ptr = ptr->next;
	}

	g_list_foreach(references, (GFunc) parentTreeGetRow, intrackCat);

	// Free the tree paths, tree row references and lists
	g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(references);
	g_list_free(rows);    
}

static void parentTreeGetRow(GtkTreeRowReference *ref, IntrackCategories *intrackCat) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *idcode;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->parentTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

    gtk_tree_model_get(model, &iter, 0, &idcode, -1); // 0 is the id column 
	
	// Safety measure if we somehow get into this popup in a multi selected tree rows, only change items with the proper code
	if(!strcmp(idcode, intrackCat->itemCode)) {
		
		// For updating parent categories from inventory_cat.c
		if(intrackCat->selectedSection == NULL) {
			gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 5, "", -1); // 5 is section id column
			gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 4, "", -1); // 4 is the section name column
		}
		else {
			gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 4, intrackCat->selectedSectionName, -1); // 2 is section column
				
			gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 5, intrackCat->selectedSection, -1); // 5 is section id column
		}
	}
	
	gtk_tree_path_free(path);
    g_free(idcode);
}

static void setupSectionTree(IntrackCategories *intrackCat) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Section Name", renderer, "text", SECTION_NAME, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SECTION_NAME); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->catTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", ID, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column.
	gtk_tree_view_column_set_sort_column_id (column, ID); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->catTree), column);
}

// Prepare to pull category data from the database
static void getSections(IntrackCategories *intrackCat) {
	GtkListStore *store;
	
	store = gtk_list_store_new (SECTION_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);

	pullSections(store);
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(intrackCat->catTree), GTK_TREE_MODEL(store));
	g_object_unref(store);
}

static int pullSections(GtkListStore *store) {
	GtkTreeIter 	iter;
	
	int i;
	int num_fields;	
	gchar *query_string;
	int query_state;
	
	// Open MYSQL connection to the database
	if(connectToServer() == 1) {
		return 1;
	}
	
    // Select the database.
    query_string = g_strdup(CATEGORY_DATABASE);	
	query_state = mysql_select_db(connection, query_string);	
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		g_free(query_string);
		mysql_close(connection);
		
		return 1;
	}	
	
	g_free(query_string);

	query_string = g_strconcat("SELECT id, label FROM `", CATEGORY_SECTION, "` ORDER BY id", NULL);

	mysql_query(connection, query_string);

	// If the connection is successful and the query returns a result then the next step is to display those results:
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);	
	
	while ((row = mysql_fetch_row(result))) {
		gchar *id, *section;
		
		for(i = 0; i < num_fields; i++) {
			if(i == 0)
				id = g_strdup(row[i]);
				
			if(i == 1)
				section = g_strdup(row[i]);
		}
		
		// We dont want to display the currently selected category so we dont parent ourself.
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, ID, id, SECTION_NAME, section, -1);
		
		g_free(id), g_free(section);
	}
	
	g_free(query_string);	
	mysql_free_result(result);
	mysql_close(connection);
	
	return 0;
}

// Mouse click event on the tree
static gboolean treeButtonPress(GtkWidget *widget, GdkEventButton *ev, IntrackCategories *intrackCat) {

    GtkTreePath *path;
    
	// if there's no path where the click occurred...
    if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(intrackCat->catTree), ev->x, ev->y, &path, NULL, NULL, NULL)) {
       	gtk_widget_set_sensitive(GTK_WIDGET(intrackCat->applyButton), FALSE);
        return FALSE;
	}

	// Keep track of the selected item
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
    gchar *rowSection, *rowSectionName;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->catTree));
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &rowSection, -1);
    gtk_tree_model_get(model, &iter, SECTION_NAME, &rowSectionName, -1);

   	gtk_widget_set_sensitive(GTK_WIDGET(intrackCat->applyButton), TRUE);
    
    g_free(intrackCat->selectedSection);
    g_free(intrackCat->selectedSectionName);

    intrackCat->selectedSection = g_strdup(rowSection);
    intrackCat->selectedSectionName = g_strdup(rowSectionName);
    
    g_free(rowSection), g_free(rowSectionName);
    
    // free our path
    gtk_tree_path_free(path);  

    return FALSE;	
}

// Keyboard key press event on the tree
static gboolean treeKeyPress(GtkWidget *widget, GdkEventKey *ev, IntrackCategories *intrackCat) {
	
    switch(ev->keyval)
    {       
        case GDK_Up:
				// Add a 1 millisecond delay before pulling the selection from the tree to get accurate selection.
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;

        case GDK_KP_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;

        case GDK_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;

        case GDK_KP_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;  
            
        case GDK_KP_Home:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;              
            
        case GDK_KP_End:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;     
            
        case GDK_Page_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;  

        case GDK_Page_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;  
            
        case GDK_KP_Page_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;  
            
        case GDK_KP_Page_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;      
            
        case GDK_Home:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;  
            
        case GDK_End:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;
    }
	
    return FALSE;	
}

static gboolean keyPressSelection(gpointer data) {
    
    IntrackCategories *intrackCat = (IntrackCategories *)data;
    
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean pathSelected;
    
    gchar *rowsection, *rowsectionname;
    
    /* Need to pull the current selected row from the treeview */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(intrackCat->catTree));

	/* Now need to pull the path and iter from the selected row */
	if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);
		pathSelected = TRUE;
	}
	else {
		pathSelected = FALSE;
		gtk_widget_set_sensitive(GTK_WIDGET(intrackCat->applyButton), FALSE);
		return FALSE;
	}
	
	if(pathSelected) {
		if(!gtk_tree_model_get_iter(model, &iter, path)) {
			/* If the iter is invalid, it means the path is invalid, so go back to the prev path and grab the iter again */
			gtk_tree_path_prev(path);
			gtk_tree_model_get_iter(model, &iter, path);
		}
			
		gtk_tree_model_get(model, &iter, ID, &rowsection, -1);
		gtk_tree_model_get(model, &iter, SECTION_NAME, &rowsectionname, -1);

		g_free(intrackCat->selectedSection);
		g_free(intrackCat->selectedSectionName);
		
		intrackCat->selectedSection = g_strdup(rowsection);
		intrackCat->selectedSectionName = g_strdup(rowsectionname);
		
		gtk_widget_set_sensitive(GTK_WIDGET(intrackCat->applyButton), TRUE);
			
		g_free(rowsection), g_free(rowsectionname);
	}
	
	
	if(pathSelected) {
		// free our path
		gtk_tree_path_free(path);
	}
		
	return FALSE;
}
