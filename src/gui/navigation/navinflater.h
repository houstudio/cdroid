#ifndef __NAVI_INFLATOR_H__
#define __NAVI_INFLATOR_H__

#include <navigation/navigatorprovider.h>
//frameworks/support/navigation/runtime/src/main/java/androidx/navigation/NavInflater.java
namespace cdroid{

class NavInflater {
    /**
     * Metadata key for defining an app's default navigation graph.
     *
     * <p>Applications may declare a graph resource in their manifest instead of declaring
     * or passing this data to each host or controller:</p>
     *
     * <pre class="prettyprint">
     *     <meta-data android:name="android.nav.graph" android:resource="@xml/my_nav_graph" />
     * </pre>
     *
     * <p>A graph resource declared in this manner can be inflated into a controller by calling
     * {@link NavController#setMetadataGraph()} or directly via {@link #inflateMetadataGraph()}.
     * Navigation host implementations should do this automatically
     * if no navigation resource is otherwise supplied during host configuration.</p>
     */
private:
    class NaviParser;
    friend NaviParser;
    Context* mContext;
    NavigatorProvider* mNavigatorProvider;
private:
    NavDestination* inflate(const std::string&name, const AttributeSet& attrs);
    void inflateArgument(NavDestination& dest, const AttributeSet& attrs);
    void inflateDeepLink(NavDestination& dest, const AttributeSet& attrs);
    void inflateAction(NavDestination& dest,  const AttributeSet& attrs);
public:
    NavInflater(Context* context,NavigatorProvider* navigatorProvider);

    /**
     * Inflates {@link NavGraph navigation graph} as specified in the application manifest.
     *
     * <p>Applications may declare a graph resource in their manifest instead of declaring
     * or passing this data to each host or controller:</p>
     *
     * <pre class="prettyprint">
     *     <meta-data android:name="android.nav.graph" android:resource="@xml/my_nav_graph" />
     * </pre>
     *
     * @see #METADATA_KEY_GRAPH
     */
    NavGraph* inflateMetadataGraph();
    /**
     * Inflate a NavGraph from the given XML resource id.
     *
     * @param graphResId
     * @return
     */
    NavGraph* inflate(const std::string& graphResId);
};
}/*endof namespace*/
#endif
