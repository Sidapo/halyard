// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-

#include "ImlUnit.h"
#include "TCommon.h"
#include "DataStore.h"

using namespace DataStore;

extern void test_DataStore (void);


//=========================================================================
//	DataStore Tests
//=========================================================================

void test_DataStore (void)
{
	Store store;
	TEST(store.CanUndo() == false);
	TEST(store.CanRedo() == false);

	MapDatum *root = store.GetRoot();
	TEST(root);

	// Test the basic value types.
	root->SetValue<IntegerDatum>("test int", 10);
	TEST(root->GetValue<IntegerDatum>("test int") == 10);
	root->SetValue<StringDatum>("test string", "foo");
	TEST(root->GetValue<StringDatum>("test string") == "foo");

	// Test the error-checking code.
	TEST_EXCEPTION(root->GetValue<IntegerDatum>("nosuch"), TException);

	// Make sure that Undo is enabled.
	TEST(store.CanUndo() == true);
	TEST(store.CanRedo() == false);

	// Test Undo by removing the "test string" key. 
	store.Undo();
	TEST(root->GetValue<IntegerDatum>("test int") == 10);
	TEST_EXCEPTION(root->GetValue<StringDatum>("test string"), TException);

	// Make sure that *both* Undo and Redo are enabled.
	TEST(store.CanUndo() == true);
	TEST(store.CanRedo() == true);

	// Test Undo by removing the "test int" key. 
	store.Undo();
	TEST_EXCEPTION(root->GetValue<IntegerDatum>("test int"), TException);
	TEST_EXCEPTION(root->GetValue<StringDatum>("test string"), TException);

	// Make sure that only Redo is enabled.
	TEST(store.CanUndo() == false);
	TEST(store.CanRedo() == true);

	// Test Redo.
	store.Redo();
	TEST(root->GetValue<IntegerDatum>("test int") == 10);
	TEST_EXCEPTION(root->GetValue<StringDatum>("test string"), TException);
	TEST(store.CanUndo() == true);
	TEST(store.CanRedo() == true);
	store.Redo();
	TEST(root->GetValue<IntegerDatum>("test int") == 10);
	TEST(root->GetValue<StringDatum>("test string") == "foo");
	TEST(store.CanUndo() == true);
	TEST(store.CanRedo() == false);
	
	// Test overwriting of Redo data.
	store.Undo();
	TEST_EXCEPTION(root->GetValue<StringDatum>("test string"), TException);
	root->SetValue<StringDatum>("test string", "bar");
	TEST(store.CanUndo() == true);
	TEST(store.CanRedo() == false);
	TEST(root->GetValue<StringDatum>("test string") == "bar");
	
	// Test clearing of Undo data.
	store.Undo();
	TEST(store.CanUndo() == true);
	TEST(store.CanRedo() == true);
	store.ClearUndoList();
	TEST(store.CanUndo() == false);
	TEST(store.CanRedo() == true);
	store.Redo();
	TEST(store.CanUndo() == true);
	TEST(store.CanRedo() == false);
	TEST(root->GetValue<StringDatum>("test string") == "bar");

	//---------------------------------------------------------------------
	// OK, we pretty much believe that Undo/Redo are correct.  Now
	// we need to test ALL the subclasses of Change.

	// Insert a new map datum to work with.
	root->Set<MapDatum>("map", new MapDatum());
	MapDatum *map = root->Get<MapDatum>("map");

	// Test CollectionDatum::SetChange on MapDatum.
	map->SetValue<StringDatum>("SetChange", "new key");
	TEST(map->GetValue<StringDatum>("SetChange") == "new key");
	store.Undo();
	TEST_EXCEPTION(map->GetValue<StringDatum>("SetChange"), TException);
	store.Redo();
	TEST(map->GetValue<StringDatum>("SetChange") == "new key");
	map->SetValue<StringDatum>("SetChange", "change value");
	TEST(map->GetValue<StringDatum>("SetChange") == "change value");
	store.Undo();
	TEST(map->GetValue<StringDatum>("SetChange") == "new key");
	store.Redo();
	TEST(map->GetValue<StringDatum>("SetChange") == "change value");

	// Test CollectionDatum::DeleteChange on MapDatum.
	TEST_EXCEPTION(map->Delete("nosuch"), TException);
	map->SetValue<StringDatum>("DeleteChange", "foo");
	TEST(map->GetValue<StringDatum>("DeleteChange") == "foo");
	map->Delete("DeleteChange");
	TEST_EXCEPTION(map->Delete("DeleteChange"), TException);
	store.Undo();
	TEST(map->GetValue<StringDatum>("DeleteChange") == "foo");
	store.Redo();
	TEST_EXCEPTION(map->Delete("DeleteChange"), TException);

	// Create a new ListDatum.
	root->Set<ListDatum>("list", new ListDatum());
	ListDatum *list = root->Get<ListDatum>("list");

	// Insert two elements.
	list->InsertValue<StringDatum>(0, "item 0");
	TEST(list->GetValue<StringDatum>(0) == "item 0");
	list->InsertValue<StringDatum>(1, "item 1");
	TEST(list->GetValue<StringDatum>(1) == "item 1");

	// Test MapDatum::SetChange and ListDatum::InsertChange on ListDatum.
	store.Undo();
	TEST(list->GetValue<StringDatum>(0) == "item 0");
	TEST_EXCEPTION(list->GetValue<StringDatum>(1), TException);
	store.Redo();
	TEST(list->GetValue<StringDatum>(0) == "item 0");
	TEST(list->GetValue<StringDatum>(1) == "item 1");
	list->InsertValue<StringDatum>(1, "item 0.5");
	TEST(list->GetValue<StringDatum>(0) == "item 0");
	TEST(list->GetValue<StringDatum>(1) == "item 0.5");
	TEST(list->GetValue<StringDatum>(2) == "item 1");
	store.Undo();
	TEST(list->GetValue<StringDatum>(0) == "item 0");
	TEST(list->GetValue<StringDatum>(1) == "item 1");
	store.Redo();
	TEST(list->GetValue<StringDatum>(0) == "item 0");
	TEST(list->GetValue<StringDatum>(1) == "item 0.5");
	TEST(list->GetValue<StringDatum>(2) == "item 1");
	list->SetValue<StringDatum>(1, "item 1/2");
	TEST(list->GetValue<StringDatum>(0) == "item 0");
	TEST(list->GetValue<StringDatum>(1) == "item 1/2");
	TEST(list->GetValue<StringDatum>(2) == "item 1");
	store.Undo();
	TEST(list->GetValue<StringDatum>(0) == "item 0");
	TEST(list->GetValue<StringDatum>(1) == "item 0.5");
	TEST(list->GetValue<StringDatum>(2) == "item 1");
	store.Undo();
	TEST(list->GetValue<StringDatum>(0) == "item 0");
	TEST(list->GetValue<StringDatum>(1) == "item 1");
	
	/*
	{
		Transaction t(store);
		root->SetValue<IntegerDatum>("test int", 20);
		TEST(root->GetValue<IntegerDatum>("test int") == 20);
	}
	*/
}
