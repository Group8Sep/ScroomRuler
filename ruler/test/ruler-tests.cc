#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "../src/ruler.hh"

namespace RulerUnitTest
{
    struct RulerTester
    {
    };
}

BOOST_AUTO_TEST_SUITE(Ruler_Tests)

void checkSignals(const Ruler::Ptr& ruler, GtkWidget* drawingArea, bool connected = true)
{
    auto mask = static_cast<GSignalMatchType>(G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA);
    guint drawID = g_signal_lookup("draw", GTK_TYPE_DRAWING_AREA);
    guint sizeAllocateID = g_signal_lookup("size-allocate", GTK_TYPE_DRAWING_AREA);
    if (connected) // Check that the ruler has connected its signal handlers
    {
        // Check that a signal handler is connected for the "draw" signal, with a pointer to ruler as data
        BOOST_CHECK(g_signal_handler_find(drawingArea, mask, drawID, 0, nullptr, nullptr, ruler.get()) != 0);
        // Check that a signal handler is connected for the "size-allocate" signal, with a pointer to ruler as data
        BOOST_CHECK(g_signal_handler_find(drawingArea, mask, sizeAllocateID, 0, nullptr, nullptr, ruler.get()) != 0);
    }
    else // Check that there are NO signal handlers connected for this ruler
    {
        BOOST_CHECK(g_signal_handler_find(drawingArea, mask, drawID, 0, nullptr, nullptr, ruler.get()) == 0);
        BOOST_CHECK(g_signal_handler_find(drawingArea, mask, sizeAllocateID, 0, nullptr, nullptr, ruler.get()) == 0);
    }
}

BOOST_AUTO_TEST_CASE(Ruler_register_creation,
    * utf::description("Testing that correct signal handlers are registered at creation."))
{
    gtk_init(nullptr, nullptr);
    // Register a new ruler with a dummy drawing area
    GtkWidget* drawingArea = gtk_drawing_area_new ();
    Ruler::Ptr ruler = Ruler::create(Ruler::HORIZONTAL, drawingArea);
    // Check that the appropriate signals are connected
    checkSignals(ruler, drawingArea);
}

BOOST_AUTO_TEST_CASE(Ruler_register_after_creation,
        * utf::description("Testing that correct signal handlers are registered when no drawing area was registered at creation."))
{
    gtk_init(nullptr, nullptr);
    // Create a dummy drawing area and a ruler, but don't register the ruler immediately
    GtkWidget *drawingArea = gtk_drawing_area_new ();
    Ruler::Ptr ruler = Ruler::create(Ruler::HORIZONTAL, nullptr);
    // Register the ruler now
    ruler->setDrawingArea(drawingArea);
    // Check that the appropriate signals are connected
    checkSignals(ruler, drawingArea);
}

BOOST_AUTO_TEST_CASE(Ruler_register_again,
         * utf::description("Testing that correct signal handlers are registered at creation."))
{
    gtk_init(nullptr, nullptr);
    // Create dummy drawing areas and register a new ruler with the first
    GtkWidget* drawingArea1 = gtk_drawing_area_new ();
    GtkWidget* drawingArea2 = gtk_drawing_area_new ();
    Ruler::Ptr ruler = Ruler::create(Ruler::HORIZONTAL, drawingArea1);
    checkSignals(ruler, drawingArea1);
    // Register the other drawing area
    ruler->setDrawingArea(drawingArea2);
    // Check that the signals are disconnected from the first drawingArea
    // and connected for the second
    checkSignals(ruler, drawingArea1, false);
    checkSignals(ruler, drawingArea2, true);
}

BOOST_AUTO_TEST_SUITE_END()