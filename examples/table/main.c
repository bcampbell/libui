// example for giving table control a workout
//
// TODO:
// editing cells
// column sorting
// other column types (numbers, colours etc)
// colouring: use background colour to indicate tech level, say

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../ui.h"
#include "db.h"

static uiTableModel *model=NULL;
static uiWindow *mainwin;
static uiButton *deleteButton;
static uiTable *table;

static DB *db = NULL;

// handler funcs to expose our data via a uiTableModel

static int handlerNumColumns(uiTableModelHandler *h, uiTableModel *m)
{
	//printf("handlerNumColumns()\n");
	return 7;
}

static uiTableValueType handlerColumnType(uiTableModelHandler *h, uiTableModel *m, int col)
{
	switch( col) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			return uiTableValueTypeString;
		default:
			return uiTableValueTypeString;
	}
}

static int handlerNumRows(uiTableModelHandler *h, uiTableModel *m)
{
	return dbCount(db);
}

static uiTableValue *handlerCellValue(uiTableModelHandler *h, uiTableModel *m, int row, int col)
{
	char buf[32];
	const Record *r = dbGet(db, row);
	switch (col) {
		case 0:
			return uiNewTableValueString(r->Name);
		case 1:
			return uiNewTableValueString(dbEconDesc[r->Economy]);
		case 2:
			return uiNewTableValueString(dbGovtDesc[r->Govt]);
		case 3:
			sprintf(buf, "%d", r->TechLevel);
			return uiNewTableValueString(buf);
		case 4:
			sprintf(buf, "%d", r->Turnover);
			return uiNewTableValueString(buf);
		case 5:
			sprintf(buf, "%dkm", r->Radius);
			return uiNewTableValueString(buf);
		case 6:
			sprintf(buf, "%.1f B", (float)r->Population/10.0f);
			return uiNewTableValueString(buf);
		default:
			return uiNewTableValueString("?");
	}
}


static void handlerSetCellValue(uiTableModelHandler *h, uiTableModel *m, int row, int col, const uiTableValue *val)
{
//	printf("handlerSetCell(row %d, col %d)\n", row,col);
}


static uiTableModelHandler handler = {
	handlerNumColumns,
	handlerColumnType,
	handlerNumRows,
	handlerCellValue,
	handlerSetCellValue
};

// event handlers

static int onClosing(uiWindow *w, void *data)
{
	uiQuit();
	return 1;
}

static int onShouldQuit(void *data)
{
	uiWindow *mainwin = uiWindow(data);

	uiControlDestroy(uiControl(mainwin));
	return 1;
}

static void onSelectionChanged(uiTable *t, void *data)
{
	int cnt;
	uiTableSelection* sel = uiTableCurrentSelection(t);

	cnt = sel->NumItems;
	uiFreeTableSelection(sel);
	printf("%d rows selected\n",cnt);
	if (cnt>0) {
		uiControlEnable(uiControl(deleteButton));
	} else {
		uiControlDisable(uiControl(deleteButton));
	}
}

// add a number of new rows
static void onAddEntries(uiButton *button, void *data)
{
	uiSpinbox* entry = (uiSpinbox*)data;
	int cnt = uiSpinboxValue(entry);
	int i;

	printf("adding %d records\n", cnt);
	for( i=0; i<cnt; ++i) {
		Record *r = dbNewRecord(); 
		int idx = dbCount(db);
		dbAppend(db,r);
		uiTableModelRowInserted(model,idx);
	}
}

static int cmpIntReverse(const void* p1, const void* p2)
{
	int a = *((const int*)p1);
	int b = *((const int*)p2);
	if (a > b)
		return -1;
	if (a < b)
		return 1;
	return 0;
}

// delete all selected rows
static void onDeleteEntries(uiButton *button, void *data)
{
	int i;
	uiTableSelection *sel = uiTableCurrentSelection(table);
	// sort the items highest to lowest, so we don't invalidate them as we delete
	qsort(sel->Items, sel->NumItems, sizeof(int), cmpIntReverse);
	for (i = 0 ; i < sel->NumItems ; i++) {
		int rowIdx = sel->Items[i];
		printf("delete row %d\n",rowIdx);
		dbDelete(db,rowIdx);
		uiTableModelRowDeleted(model, rowIdx);
	}
	uiFreeTableSelection(sel);
}


static uiTable *makeTable(void)
{
	uiTableParams params;
	uiTable* t;

	model = uiNewTableModel(&handler);

	params.Model = model;
	params.RowBackgroundColorModelColumn = -1;
	params.MultiSelect = 1;

	t = uiNewTable(&params);
	uiTableAppendTextColumn(t, "Name", 0, -1, NULL);
	uiTableAppendTextColumn(t, "Economy", 1, -1, NULL);
	uiTableAppendTextColumn(t, "Government", 2, -1, NULL);
	uiTableAppendTextColumn(t, "Tech Level", 3, -1, NULL);
	uiTableAppendTextColumn(t, "Turnover", 4, -1, NULL);
	uiTableAppendTextColumn(t, "Radius", 5, -1, NULL);
	uiTableAppendTextColumn(t, "Population", 6, -1, NULL);

	uiTableOnSelectionChanged(t, onSelectionChanged, NULL);
	return t;
}

static uiControl *makeLayout(void)
{
	uiBox *vbox;
	uiSpinbox *entry;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox,1);

	table = makeTable();
	uiBoxAppend(vbox,uiControl(table),1);
	{
		uiGroup* group = uiNewGroup("Add rows");
		uiGroupSetMargined(group, 1);
		{
			uiBox* hbox = uiNewHorizontalBox();
			uiBoxSetPadded(hbox,1);
			uiBoxAppend(hbox, uiControl(uiNewLabel("number of rows")), 0);
			entry = uiNewSpinbox(1,100000);
			uiSpinboxSetValue(entry, 10);
			uiBoxAppend(hbox, uiControl(entry),0);
			uiButton* addButton = uiNewButton("Add");

			uiButtonOnClicked(addButton, onAddEntries, entry);
			uiBoxAppend(hbox, uiControl(addButton),0);
			uiGroupSetChild(group, uiControl(hbox));
		}
		uiBoxAppend(vbox, uiControl(group), 0);

		// delete button
		{
			deleteButton = uiNewButton("Delete selected rows");
			uiControlDisable(uiControl(deleteButton));
			uiButtonOnClicked(deleteButton, onDeleteEntries, 0);
			uiBoxAppend(vbox, uiControl(deleteButton), 0);
		}
	}

	return uiControl(vbox);
}


int main(void)
{
#ifdef _WIN32
	setbuf(stdout, NULL);
#endif

	uiInitOptions options;
	const char *err;

	memset(&options, 0, sizeof (uiInitOptions));
	err = uiInit(&options);
	if (err != NULL) {
		fprintf(stderr, "error initializing libui: %s", err);
		uiFreeInitError(err);
		return 1;
	}

	db = dbNewDB();
	int i;
	for( i=0; i<20; ++i) {
		Record* r = dbNewRecord();
		dbAppend(db,r);
	}

	mainwin = uiNewWindow("libui Table Example", 640, 480, 1);
	uiWindowOnClosing(mainwin, onClosing, NULL);
	uiOnShouldQuit(onShouldQuit, mainwin);

	uiControl* layout = makeLayout();

	uiWindowSetChild(mainwin, layout);
	uiWindowSetMargined(mainwin, 1);

	uiControlShow(uiControl(mainwin));
	uiMain();

	if (db) {
		dbFreeDB(db);
	}

	return 0;
}

