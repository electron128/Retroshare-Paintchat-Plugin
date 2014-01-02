#include "paintchatplugin.h"

#include "gui/paintchatpopupchatdialog.h"

extern "C" {

    // This is *the* functions required by RS plugin system to give RS access to the plugin.
    // Be careful to:
    // - always respect the C linkage convention
    // - always return an object of type RsPlugin*
    //
    void *RETROSHARE_PLUGIN_provide()
    {
        static PaintChatPlugin *p = new PaintChatPlugin() ;

        return (void*)p ;
    }

    // This symbol contains the svn revision number grabbed from the executable.
    // It will be tested by RS to load the plugin automatically, since it is safe to load plugins
    // with same revision numbers, assuming that the revision numbers are up-to-date.
    //
    uint32_t RETROSHARE_PLUGIN_revision = SVN_REVISION_NUMBER ;

    // This symbol contains the svn revision number grabbed from the executable.
    // It will be tested by RS to load the plugin automatically, since it is safe to load plugins
    // with same revision numbers, assuming that the revision numbers are up-to-date.
    //
    uint32_t RETROSHARE_PLUGIN_api = RS_PLUGIN_API_VERSION ;
}

PaintChatPlugin::PaintChatPlugin():mService(NULL){
    std::cerr<<"PaintChatPlugin::PaintChatPlugin()"<<std::endl;
}
PaintChatPlugin::~PaintChatPlugin(){
    std::cerr<<"PaintChatPlugin::~PaintChatPlugin()"<<std::endl;
}

std::string PaintChatPlugin::getShortPluginDescription()const{
    return std::string("this is the PaintChatPlugin");
}
std::string PaintChatPlugin::getPluginName()const{
    return std::string("PaintChat");
}
void PaintChatPlugin::getPluginVersion(int &major, int &minor, int &svn_rev)const{
    major=0;
    minor=0;
    svn_rev=0;
}
void PaintChatPlugin::setInterfaces(RsPlugInInterfaces &interfaces){

}
void PaintChatPlugin::setPlugInHandler(RsPluginHandler *pgHandler){
    std::cerr<<"PaintChatPlugin::setPlugInHandler()"<<std::endl;
    mPlugInHandler = pgHandler;
}

RsPQIService *PaintChatPlugin::rs_pqi_service()const{
    std::cerr<<"PaintChatPlugin::rs_pqi_service()"<<std::endl;
    if(mService==NULL){
        std::cerr<<"PaintChatPlugin::rs_pqi_service(): creating new p3PaintChatService"<<std::endl;
        mService=new p3PaintChatService(mPlugInHandler);
        paintChatService=mService;
    }
    return mService;
}

ChatWidgetHolder *PaintChatPlugin::qt_get_chat_widget_holder(ChatWidget *chatWidget) const{
    return new PaintChatPopupChatDialog(chatWidget);
}
