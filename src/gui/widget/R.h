#ifndef __CDROID_R_H__
#define __CDROID_R_H__
namespace cdroid{
class R{
public:
   enum id{
       background=0,
       progress,
       secondaryProgress,
       toggle,
       text1,
       numberpicker_input,//for numberpicker
       increment,//for numberpicker
       decrement,//for numberpicker
   /**for dialog */
       parentPanel,
       topPanel,
       contentPanel,
       buttonPanel,
       customPanel,
       custom,
 
       textSpacerNoButtons,
       titleDividerNoCustom,
       titleDivider,
       titleDividerTop,
       textSpacerNoTitle,
       title_template,
       icon,
       alertTitle,
   };
};
}
#endif
