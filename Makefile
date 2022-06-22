# Michael Rajotte
# Copyright 2010 - 2014 Michael Rajotte <michael@michaelrajotte.com>
# Makefile

CC := gcc
INCLUDE := -I/usr/local/include
COMMON:= inventory.c export.c import.c partSales.c accounts.c accounts_view.c saleView.c warranty.c sales.c inventory_add.c partSales_add.c systems.c systems_add.c view_item.c casl.c inventory_cat.c inventory_cat_items.c category_popup.c section_popup.c inventory_hidemenu.c
LIBS:= -lz -L/usr/include/mysql -lmysqlclient -I/usr/include/mysql
PKGCONFIG = `(pkg-config --cflags --libs gtk+-2.0 gmodule-2.0 gthread-2.0)`

barcode: milparts.c milparts.h systray.h messages.h configuration.h functions.h settings.h inventory.c inventory.h export.c export.h import.c import.h partSales.c partSales.h calendar.h accounts.c accounts.h accounts_structs.h accounts_view.c accounts_view.h saleView.c saleView.h warranty.c warranty.h sales.c sales.h inventory_add.c inventory_add.h partSales_add.c partSales_add.h systems.c systems.h systems_add.c systems_add.h view_item.c view_item.h casl.c casl.h inventory_cat.c inventory_cat.h inventory_cat_structs.h inventory_cat_items.c inventory_cat_items.h category_popup.c category_popup.h section_popup.c section_popup.h inventory_hidemenu.c inventory_hidemenu.h
	$(CC) -o milparts milparts.c $(COMMON) -lm $(INCLUDE) $(ONEDIM) $(LIBS) $(PKGCONFIG)
