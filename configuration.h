//      configuration.h
//      Copyright 2010 Michael Rajotte <michael@michaelrajotte.com>
// 		Loads the configuration file.

/*
TODO:
Fix updateConfiguration() where it goes tables=items. Remove that and keep the items tables name into the messages.h
*/

int updateConfiguration(GtkWidget *widget, gpointer parentWindow) {

	FILE *file;
	gchar *printValue = NULL;
	
	// open/create the file for writing.
	file = fopen(CONFIG_FILE,"w");
	
	printValue = g_strconcat("server=", gtk_entry_get_text(GTK_ENTRY(configMYSQLServer)), NULL);
	printValue = g_strconcat(printValue, "\n", NULL);
	fprintf(file, "%s", printValue);
	
	printValue = g_strconcat("login=", gtk_entry_get_text(GTK_ENTRY(configMYSQLUsername)), NULL);
	printValue = g_strconcat(printValue, "\n", NULL);
	fprintf(file, "%s", printValue);
	
	printValue = g_strconcat("password=", gtk_entry_get_text(GTK_ENTRY(configMYSQLPassword)), NULL);
	printValue = g_strconcat(printValue, "\n", NULL);
	fprintf(file, "%s", printValue);
	
	printValue = g_strconcat("database=", gtk_entry_get_text(GTK_ENTRY(configMYSQLDatabase)), NULL);
	printValue = g_strconcat(printValue, "\n", NULL);
	fprintf(file, "%s", printValue);
	
	fprintf(file, "%s", "tables=priceListMilParts");
	
	// Close the file.
	fclose(file); 	
	
	// Output query status to the output box in the software
	//printMessage(CONFIG_UPDATE);
	showMessageDialog("Configuration Updated.", window);
	
	
	// Reload the new settings
	loadConfigurationSettings(NULL);		
	
	return 0;
}


void configureSettings(GtkWidget *widget, gpointer parentWindow) {
	
	GtkWidget *inputWindow;

	GtkWidget *table;

	GtkWidget *label1;
	GtkWidget *label2;
	GtkWidget *label3;
	GtkWidget *label4;

	GtkWidget *sendButton;
	GtkWidget *cancelButton;
	
	// Create and setup the new input popup window.
	inputWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(inputWindow), 340, 210);
	gtk_window_set_title(GTK_WINDOW(inputWindow), "Configuration Menu");
	gtk_container_set_border_width(GTK_CONTAINER(inputWindow), 10);
	gtk_window_set_icon(GTK_WINDOW(inputWindow), create_pixelbuffer_icon(PROGRAM_ICON));
	
	// Set the new inputWindow so it is above the original parent window of the program.
	gtk_window_set_transient_for(GTK_WINDOW(inputWindow), GTK_WINDOW(parentWindow));

	// Create a table to layout and organize the text and entry fields.
	table = gtk_table_new(4, 2, FALSE);
	gtk_container_add(GTK_CONTAINER(inputWindow), table);
	
	gtk_window_set_position(GTK_WINDOW(inputWindow), GTK_WIN_POS_CENTER);    

	label1 = gtk_label_new("Database Server");
	label2 = gtk_label_new("Login Name");
	label3 = gtk_label_new("Password");
	label4 = gtk_label_new("Database Name");
	
	gtk_table_attach(GTK_TABLE(table), label1, 0, 1, 0, 1, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label2, 0, 1, 1, 2, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label3, 0, 1, 2, 3, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label4, 0, 1, 3, 4, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

	configMYSQLServer = gtk_entry_new();
	configMYSQLUsername = gtk_entry_new();
	configMYSQLPassword = gtk_entry_new();
	configMYSQLDatabase = gtk_entry_new();

	sendButton = gtk_button_new_with_label("Ok");
	gtk_widget_set_size_request(sendButton, 80, 30 );
		
	cancelButton = gtk_button_new_with_label("Cancel");
	gtk_widget_set_size_request(cancelButton, 80, 30 );	
	
	gtk_table_set_homogeneous(GTK_TABLE(table), FALSE);

	gtk_table_attach(GTK_TABLE(table), configMYSQLServer, 1, 2, 0, 1, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), configMYSQLUsername, 1, 2, 1, 2, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), configMYSQLPassword, 1, 2, 2, 3, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), configMYSQLDatabase, 1, 2, 3, 4, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

	gtk_table_attach(GTK_TABLE(table), sendButton, 1, 2, 5, 6,  GTK_SHRINK, GTK_SHRINK, 1, 5);
	gtk_table_attach(GTK_TABLE(table), cancelButton, 0, 1, 5, 6, GTK_SHRINK, GTK_SHRINK, 1, 5);

	// Set the positions of my buttons so they are spaced and look good.
	gtk_widget_set_uposition(sendButton, 215, 170);
	gtk_widget_set_uposition(cancelButton, 125, 170);
	
	// Set the settings values into the entry text boxes. Only load them if coming from a menu button since the file exists already, else leave them blank.
	if(widget != NULL) {
		gtk_entry_set_text(GTK_ENTRY(configMYSQLServer), mysqlServer);
		gtk_entry_set_text(GTK_ENTRY(configMYSQLUsername), mysqlUsername);
		gtk_entry_set_text(GTK_ENTRY(configMYSQLPassword), mysqlPassword);
		gtk_entry_set_text(GTK_ENTRY(configMYSQLDatabase), mysqlDatabase);
	}
	
	// Set the password entry box to hide the text.
	gtk_entry_set_visibility(GTK_ENTRY(configMYSQLPassword), FALSE);
	// Set the password entry box to show the * char instead of it's true text result, to keep the password hidden from public viewing.
	gtk_entry_set_invisible_char(GTK_ENTRY(configMYSQLPassword), 0x25cf); //0x25cf is the * invis character.

	gtk_widget_show(table);

	gtk_widget_show(label1);
	gtk_widget_show(label2);
	gtk_widget_show(label3);
	gtk_widget_show(label4);

	gtk_widget_show(configMYSQLServer);
	gtk_widget_show(configMYSQLUsername);
	gtk_widget_show(configMYSQLPassword);
	gtk_widget_show(configMYSQLDatabase);
	
	gtk_widget_show(sendButton);
	gtk_widget_show(cancelButton);

	// Close the window (destroy's the widget)
	g_signal_connect(G_OBJECT(cancelButton), "clicked", G_CALLBACK(destroyWidget), inputWindow);

	g_signal_connect(G_OBJECT(sendButton), "clicked", G_CALLBACK(updateConfiguration), inputWindow);
	g_signal_connect(G_OBJECT(sendButton), "clicked", G_CALLBACK(destroyWidget), inputWindow);
	
	gtk_widget_show(inputWindow);
}


char *scanConfigurationFile(FILE *tempFile) {
	
	int i=0, writeData=0, arrayCounter=0;
	char outputData[80];

	char *returnData = malloc(80);
	
	fscanf(tempFile, "%s", outputData);

	while(outputData[i]) {
		
		if (outputData[i++] == '=') {
				writeData = 1;
		}
		
		if(writeData == 1) {
			returnData[arrayCounter] = outputData[i];
			++arrayCounter;
		}
	}	
	
	return returnData;
}

int loadConfigurationSettings(gpointer parentWindow) {
	
	FILE *file;
	// open the file.
	file = fopen(CONFIG_FILE,"r"); 
	
	// If does not exist or can not load it, bring up configuration popup menu and print out error report.
	if(file==NULL) {
		printMessage(ERROR_CONFIG_LOAD, parentWindow);

		// Load configuration menu
		configureSettings(NULL, parentWindow);
		return 1;
	}
	
	mysqlServer = scanConfigurationFile(file);
	mysqlUsername = scanConfigurationFile(file);
	mysqlPassword = scanConfigurationFile(file);
	mysqlDatabase = scanConfigurationFile(file);
	mysqlTables = scanConfigurationFile(file);
	
	// Close the file.
	fclose(file); 	
	
	//printMessage(CONFIG_LOADED);
	
	// Check network connection to server
	checkNetworkConnection();
	
	return 0;
}
