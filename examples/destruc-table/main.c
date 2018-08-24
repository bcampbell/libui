// minimal example to show windows table bug:
//
// After uiTableModelRowInserted() is called on a visible table,
// the handler CellValue() function is called with an invalid
// row number.
//
// This example shows a mocked-up dataset, and has an "add"
// button below which adds a single row. Clicking "add" will
// trigger the bug.
//
// See the assert() in handlerCellValue() below.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ui.h>

static uiTableModel *model=NULL;
static uiWindow *mainwin;
static uiButton *deleteButton;
static uiTable *table;

static int numrows = 5;

static int handlerNumColumns(uiTableModelHandler *h, uiTableModel *m)
{
	return 1;
}

static uiTableValueType handlerColumnType(uiTableModelHandler *h, uiTableModel *m, int col)
{
	return uiTableValueTypeString;
}

static int handlerNumRows(uiTableModelHandler *h, uiTableModel *m)
{
	return numrows;
}

static uiTableValue *handlerCellValue(uiTableModelHandler *h, uiTableModel *m, int row, int col)
{
	char buf[32];
	// THIS WILL TRIGGER after uiTableModelRowInserted() is called
	assert(row<numrows);
	sprintf(buf, "cell %d,%d\n", row,col);
	return uiNewTableValueString(buf);
}

static void handlerSetCellValue(uiTableModelHandler *h, uiTableModel *m, int row, int col, const uiTableValue *val)
{
}

static uiTableModelHandler handler = {
	handlerNumColumns,
	handlerColumnType,
	handlerNumRows,
	handlerCellValue,
	handlerSetCellValue
};

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

static void onAdd(uiButton *button, void *data)
{
	++numrows;
	uiTableModelRowInserted(model,numrows-1);
}

static uiTable *makeTable(void)
{
	uiTableParams params;
	uiTable* t;
	model = uiNewTableModel(&handler);
	params.Model = model;
	params.RowBackgroundColorModelColumn = -1;
	t = uiNewTable(&params);
	uiTableAppendTextColumn(t, "Col 0", 0, -1, NULL);
	return t;
}

static uiControl *makeLayout(void)
{
	uiBox *vbox;
	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox,1);
	table = makeTable();
	uiBoxAppend(vbox,uiControl(table),1);

	uiButton* but = uiNewButton("Add");
	uiButtonOnClicked(but, onAdd, 0);
	uiBoxAppend(vbox, uiControl(but),0);
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
	mainwin = uiNewWindow("destruc-table", 640, 480, 1);
	uiWindowOnClosing(mainwin, onClosing, NULL);
	uiOnShouldQuit(onShouldQuit, mainwin);
	uiControl* layout = makeLayout();
	uiWindowSetChild(mainwin, layout);
	uiWindowSetMargined(mainwin, 1);
	uiControlShow(uiControl(mainwin));
	uiMain();
	return 0;
}

