#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "../src/ruler.hh"

BOOST_AUTO_TEST_SUITE(Ruler_Tests)

void checkSignals(const Ruler::Ptr& ruler, GtkWidget* drawingArea)
{
    // [TODO] Also check correct function is connected!
    auto mask = static_cast<GSignalMatchType>(G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA);
    guint drawID = g_signal_lookup("draw", GTK_TYPE_DRAWING_AREA);
    guint sizeAllocateID = g_signal_lookup("size-allocate", GTK_TYPE_DRAWING_AREA);
    // Check that a signal handler is connected for the "draw" signal, with a pointer to ruler as data
    BOOST_CHECK(g_signal_handler_find(drawingArea, mask, drawID, 0, nullptr, nullptr, ruler.get()) != 0);
    // Check that a signal handler is connected for the "size-allocate" signal, with a pointer to ruler as data
    BOOST_CHECK(g_signal_handler_find(drawingArea, mask, sizeAllocateID, 0, nullptr, nullptr, ruler.get()) != 0);
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

BOOST_AUTO_TEST_SUITE_END()