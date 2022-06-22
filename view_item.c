//      view_item.c
//      Copyright 2010-2014 Michael Rajotte <michael@michaelrajotte.com>
// 		For display information about a item in inventory (Stock, price, cost, picture (if available), etc).

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
#include "view_item.h"

/*
TODO:
(1). Currently only supports jpg and png and a few other formats. GIF does not load??
(2). Build a right click on image, save as to allow saving the image to your hard drive.
(3). Add some nice background colouring to the gui widgets.

				  ITEM IMAGE HERE

					STATS BAR
[ Stock: #  Category: cat_name   Manufacturer: man_name ]
[ Cost: $   Cost Avg: $    Price: $   Last Sold: time_stamp ]
[ Duty: %   Country of Origin: Country ]


									CLOSE BUTTON

*/

void loadViewItem(GtkWidget *parentWindow, gchar *barcode) {
	GtkBuilder *builder;
    builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, VIEW_ITEM_FILE, NULL);
	
	ViewContainer *viewItem;
	viewItem = (ViewContainer*) g_malloc (sizeof (ViewContainer)); // possible memory leak?
	
	viewItem->id = g_strdup(barcode);
	
	viewItem->mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "viewWindow"));
	g_signal_connect(viewItem->mainWindow, "destroy", G_CALLBACK(freeMemory), viewItem);
	
	// Setup the top level window
    gtk_window_set_title (GTK_WINDOW(viewItem->mainWindow), "View Item");
	gtk_window_set_modal(GTK_WINDOW(viewItem->mainWindow), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(viewItem->mainWindow), TRUE);
	//gtk_widget_set_size_request(viewItem->mainWindow, 650, 650);
	gtk_window_set_deletable(GTK_WINDOW(viewItem->mainWindow), FALSE);
	gtk_window_set_position(GTK_WINDOW(viewItem->mainWindow), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(viewItem->mainWindow), 0);
	gtk_window_set_transient_for(GTK_WINDOW(viewItem->mainWindow), GTK_WINDOW(parentWindow));
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(viewItem->mainWindow), TRUE);
	
	// Setup the buttons
	viewItem->closeButton = GTK_WIDGET(gtk_builder_get_object(builder, "closeButton"));
	g_signal_connect(viewItem->closeButton, "clicked", G_CALLBACK(destroyWindow), viewItem);
	
	// Upload Button
	GtkWidget	*uploadButton;
	uploadButton = GTK_WIDGET(gtk_builder_get_object(builder, "uploadButton"));
	g_signal_connect(uploadButton, "clicked", G_CALLBACK(send_image), viewItem);
		
	viewItem->imageButton = GTK_WIDGET(gtk_builder_get_object(builder, "imageButton"));
	
	// Setup the item information labels
	viewItem->stockLabel = GTK_WIDGET(gtk_builder_get_object(builder, "stockLabel"));
	viewItem->catLabel = GTK_WIDGET(gtk_builder_get_object(builder, "catLabel"));
	viewItem->manLabel = GTK_WIDGET(gtk_builder_get_object(builder, "manLabel"));
	viewItem->costLabel = GTK_WIDGET(gtk_builder_get_object(builder, "costLabel"));
	viewItem->costAvgLabel = GTK_WIDGET(gtk_builder_get_object(builder, "costAvgLabel"));
	viewItem->priceLabel = GTK_WIDGET(gtk_builder_get_object(builder, "priceLabel"));
	viewItem->weightLabel = GTK_WIDGET(gtk_builder_get_object(builder, "weightLabel"));
	//viewItem->dutyLabel = GTK_WIDGET(gtk_builder_get_object(builder, "dutyLabel"));
	//viewItem->countryLabel = GTK_WIDGET(gtk_builder_get_object(builder, "countryLabel"));
	viewItem->soldLabel = GTK_WIDGET(gtk_builder_get_object(builder, "soldLabel"));
	viewItem->nameLabel = GTK_WIDGET(gtk_builder_get_object(builder, "nameLabel"));
	viewItem->codeLabel = GTK_WIDGET(gtk_builder_get_object(builder, "codeLabel"));
	viewItem->itemImage = GTK_WIDGET(gtk_builder_get_object(builder, "itemImage"));
	
	// Load the item information data
	if(checkIfExist(barcode) == 1)
		loadItemInfo(viewItem, barcode, TRUE);
	else
		loadItemInfo(viewItem, barcode, FALSE);	
	
	gtk_widget_show_all(viewItem->mainWindow);
    g_object_unref(G_OBJECT(builder));
}

static void destroyWindow(GtkWidget *widget, ViewContainer *viewItem) {
	gtk_widget_destroy(GTK_WIDGET(viewItem->mainWindow));
	
	//freeMemory(NULL, viewItem);
}

static void freeMemory(GtkWidget *widget, ViewContainer *viewItem) {
	if(widget)
		destroyWindow(NULL, viewItem);

	gtk_image_clear(GTK_IMAGE(viewItem->itemImage));
	g_free(viewItem->id);
	g_free(viewItem);
}

// Loads the item information to display on the page. Such as price, cost, cost average, etc
static int loadItemInfo(ViewContainer *viewItem, gchar *itemCode, gboolean notExist) {
	// Connection Error
	if(connectToServer() == 1)
		return 1;
	
	gchar *query_string;
	query_string = g_strconcat("SELECT stock, category, manufacturer, cost, costAvg, price, lastSold, description, weight, partNo FROM ", mysqlTables, " WHERE id = '", itemCode, "'", NULL);
	
	int query_state;
	query_state = mysql_query(connection, query_string);
	
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
	}

	g_free(query_string);
	
	result = mysql_store_result(connection);

	int num_fields = mysql_num_fields(result);
	int i = 0;

	while(row = mysql_fetch_row(result)) {
		for(i = 0; i < num_fields; i++) {
			if(i == 0)
				gtk_label_set_text(GTK_LABEL(viewItem->stockLabel), row[i]);

			if(i == 1)
				gtk_label_set_text(GTK_LABEL(viewItem->catLabel), row[i]);

			if(i == 2)
				gtk_label_set_text(GTK_LABEL(viewItem->manLabel), row[i]);
					
			if(i == 3) {
				float cost;
				gchar *costTemp;
				cost = atof(row[i]);
				costTemp = g_strdup_printf("%.2f", cost);
				gtk_label_set_text(GTK_LABEL(viewItem->costLabel), costTemp);
			
				g_free(costTemp);
			}
				
			if(i == 4) {
				float costAvg;
				gchar *costAvgTemp;
				costAvg = atof(row[i]);
				costAvgTemp = g_strdup_printf("%.2f", costAvg);
				gtk_label_set_text(GTK_LABEL(viewItem->costAvgLabel), costAvgTemp);
					
				g_free(costAvgTemp);					
			}
				
			if(i == 5) {
				float price;
				gchar *priceTemp;
				price = atof(row[i]);
				priceTemp = g_strdup_printf("%.2f", price);
				gtk_label_set_text(GTK_LABEL(viewItem->priceLabel), priceTemp);
				
				g_free(priceTemp);					
			}
			
			if(i == 6)
				gtk_label_set_text(GTK_LABEL(viewItem->soldLabel), row[i]);			

			if(i == 7)
				gtk_label_set_text(GTK_LABEL(viewItem->nameLabel), row[i]);
								
			if(i == 8)
				gtk_label_set_text(GTK_LABEL(viewItem->weightLabel), row[i]);
				
			if(i == 9)
				gtk_label_set_text(GTK_LABEL(viewItem->codeLabel), row[i]);
		}
	}
	
	// gtk_label_set_text(GTK_LABEL(viewItem->codeLabel), itemCode);
	
	mysql_free_result(result);
	mysql_close(connection);
		
	getItemImage(viewItem, itemCode);

	return 0;
}

// Load the item image
// Only supports jpeg at the moment. Need to build support for all image formats
static int getItemImage(ViewContainer *viewItem, gchar *itemCode) {

	if(connectToServer() == 1) {
		g_print("connection to server error -> loadItemImage()\n");
		printMessage("Connection to server error -> checkout_view.loadItemImage()", viewItem->mainWindow);
	}
	
	gchar *query_string = g_strconcat("SELECT imageData FROM ", mysqlTables, " WHERE id = '", itemCode, "'", NULL);
	mysql_real_connect(connection,mysqlServer,mysqlUsername,mysqlPassword,mysqlDatabase, 0, NULL, 0);
	mysql_query(connection, query_string);
	g_free(query_string);
	
	unsigned long *lengths;
	result = mysql_store_result(connection);
	row = mysql_fetch_row(result);
	lengths = mysql_fetch_lengths(result);

	int width, height;

	GdkPixbuf *pixbuffer;
	//GdkPixbuf *scaled;
	GdkPixbufLoader *loader;
	
	loader = gdk_pixbuf_loader_new();
	
	if(loader != NULL) {
		gdk_pixbuf_loader_get_animation(loader);

		if(gdk_pixbuf_loader_write (loader, row[0], lengths[0], NULL) && gdk_pixbuf_loader_close (loader, NULL)) {
			pixbuffer = gdk_pixbuf_loader_get_pixbuf(loader);

			if(pixbuffer != NULL) {
				width = gdk_pixbuf_get_width(pixbuffer);
				height = gdk_pixbuf_get_height(pixbuffer);
				
				if(height > 140 && height >= width) {
					GdkPixbuf *scaled;
					
					float modheight = 140;
					float diff = height / modheight;
					float modwidth = width / diff;
					
					// Extra check to make sure width is not over the boundaries
					if(modwidth > 200) {
						diff = modwidth / 200;
						modwidth = 200;
						modheight = modheight / diff;
					}
					
					scaled = gdk_pixbuf_scale_simple(pixbuffer, modwidth, modheight, GDK_INTERP_BILINEAR);
					g_object_unref(pixbuffer);
					pixbuffer = scaled;
				}
				else if(width > 200 && width >= height) {
					GdkPixbuf *scaled;
					
					float modwidth = 200;
					float diff = width / modwidth;
					float modheight = height / diff;
		
					// Extra check to make sure the height is not over the boundaries
					if(modheight > 140) {
						diff = modheight / 140;
						modheight = 140;
						modwidth = modwidth / diff;
					}
			
					scaled = gdk_pixbuf_scale_simple(pixbuffer, modwidth, modheight, GDK_INTERP_BILINEAR);
					g_object_unref(pixbuffer);
					pixbuffer = scaled;
				}
				gtk_image_clear(GTK_IMAGE(viewItem->itemImage));
				gtk_image_set_from_pixbuf(GTK_IMAGE(viewItem->itemImage), pixbuffer);

				g_object_unref(pixbuffer);
				
				//if(scaled != NULL)
					//g_object_unref(scaled);
			}
		}
		//if(loader != NULL)
			//g_object_unref(loader); // Memory leak here ??
	}

	mysql_free_result(result);
	mysql_close(connection);
	
	return 0;
}

static int checkIfExist(gchar *barcode) {
	if(connectToServer() == 1)
		return 1;
	
	gchar *query_string;
	query_string = g_strconcat("SELECT id FROM ", mysqlTables, " WHERE id = '", barcode, "'", NULL);
	
	int query_state;
	query_state = mysql_query(connection, query_string);
	
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
	}
	g_free(query_string);

	// Get the query result data.
	result = mysql_store_result(connection);
	
	// If the item is not in the database, then stop and return back.
	if (!mysql_fetch_row(result)) {
		mysql_free_result(result); // Free up some memory.
		mysql_close(connection);
	
		return 1;
	}
	mysql_free_result(result); // Free up some memory.
	mysql_close(connection);	
	
	return 0;
}

static int send_image(GtkWidget *widget, ViewContainer *viewItem) {
	if(connectToServer() == 1)
		return 1;
	
	// Begin file processing.
	gchar *fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(viewItem->imageButton));
	
	// If file exists locally, time to upload it.
	// This should handle up to 4gig files. Try to keep files under 100MB for performance/memory on the server. Ideally stay under 16MB as that is the max packet size for a MYSQL connection??
	if(fileName != NULL && strlen(fileName) > 0) {
		FILE *fp;
		fp = fopen(fileName, "rb");
		
		if (fp == NULL) {
			g_print("cannot open image file\n");
			printMessage("cannot open image file", viewItem->mainWindow); 
			
			g_free(fileName);
			mysql_close(connection);
			return 1;
		}
		
		// Move pointer to end of the file. About to determine the size of the image.
		fseek(fp, 0, SEEK_END);

		if (ferror(fp)) {
			g_print("fseek() failed\n");
			printMessage("fseek() failed", viewItem->mainWindow);    

			int r = fclose(fp);

			if (r == EOF) {
				g_print("cannot close file handler\n");
				printMessage("cannot close file handler", viewItem->mainWindow);          
			}    
		  
			g_free(fileName);
			mysql_close(connection);
			return 1;
		}
		
		// Return the number of bytes from the beginning of the file, ie: determine it's file size.
		int flen = ftell(fp);
		
		if (flen == -1) {
			g_print("error occured");
			
			int r = fclose(fp);

			if (r == EOF) {
				g_print("cannot close file handler\n");
				printMessage("cannot close file handler", viewItem->mainWindow);          
			}
      
			g_free(fileName);
			mysql_close(connection);	
			return 1;      
		}
		
		fseek(fp, 0, SEEK_SET);
		 
		if (ferror(fp)) {
			g_print("fseek() failed\n");
			printMessage("fseek() failed", viewItem->mainWindow);
			
			int r = fclose(fp);

			if (r == EOF) {
				g_print("cannot close file handler\n");
				printMessage("cannot close file handler", viewItem->mainWindow); 
			}    
			
			g_free(fileName);
			mysql_close(connection);	  
			return 1;
		}			
		
		char data[flen+1];
		
		// Read the file data and store it into a data array
		int size = fread(data, 1, flen, fp);
		  
		if (ferror(fp)) {
			g_print("fread() failed\n");
			printMessage("fread() failed", viewItem->mainWindow);
			
			int r = fclose(fp);

			if (r == EOF) {
				g_print("cannot close file handler\n");
				printMessage("cannot close file handler", viewItem->mainWindow);
			}
			
			g_free(fileName);	 
			mysql_close(connection); 
			return 1;      
		}
		
		int r = fclose(fp);

		if (r == EOF) {
			g_print("cannot close file handler\n");
			printMessage("cannot close file handler", viewItem->mainWindow);
		}
		
		//char chunk[2*size+1];
		char *chunk = (char *) malloc(sizeof(int) * (2*size+1)); // Allocate memory, required for large images as the heap space runs out.

		mysql_real_escape_string(connection, chunk, data, size);
		
		char *st = g_strconcat("UPDATE ", mysqlTables, " SET imageData = '%s' WHERE id='", viewItem->id, "'", NULL);

		size_t st_len = strlen(st);
		
		char *query = (char *) malloc(sizeof(int) * (st_len + (2*size+1))); // Allocate memory, required for large images as the heap space runs out.

		int len = snprintf(query, st_len + 2*size+1, st, chunk);
		
		mysql_real_query(connection, query, len);		
		
		// Free up memory
		g_free(st);
		free(chunk), free(query);
		
		getItemImage(viewItem, viewItem->id);
		gtk_file_chooser_unselect_filename(GTK_FILE_CHOOSER(viewItem->imageButton), fileName);
	}
	else {
		printMessage("Select a image to upload", viewItem->mainWindow);
		mysql_close(connection);
		return 1;
	}

	g_free(fileName);		
		
	mysql_close(connection);
	
	return 0;
}
