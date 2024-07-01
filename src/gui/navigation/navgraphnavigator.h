#ifndef __NAVGRAPH_NAVIGATOR_H__
#define __NAVGRAPH_NAVIGATOR_H__
#include <navigation/navigator.h>
#include <navigation/navgraph.h>

namespace cdroid{
class NavGraphNavigator :public Navigator{
private:
    Context* mContext;

    /**
     * Construct a Navigator capable of routing incoming navigation requests to the proper
     * destination within a {@link NavGraph}.
     * @param context
     */
public:
    NavGraphNavigator(Context* context);

    /**
     * Creates a new {@link NavGraph} associated with this navigator.
     * @return
     */
    NavGraph* createDestination() override;
    void navigate(/*@NonNull NavGraph*/NavDestination* destination, /*@Nullable*/Bundle* args,
            /*@Nullable*/ NavOptions* navOptions) override;

    bool popBackStack() override;
};
}/*endof namespace*/
#endif /*__NAVGRAPH_NAVIGATOR_H__*/


