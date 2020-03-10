#ifndef _CONF_HANDLER_H_
#define _CONF_HANDLER_H_
//-----------------------------------------------------------------------------
///
/// \brief  conf handler
///
///         see implemention for further details
///
/// \date   [20161129] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <string>
#include <cstddef>
#include <vector>
#include <libxml++/document.h>
#include <libxml++/parsers/domparser.h>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <glibmm/object.h>

//---Own------------------------------

#ifdef DEBUG_CLASSES
    #include "debug.h"
#endif
#include "post_processor.h"
#include "input_processor.h"
#include "serial_protocol.h"
#include "lan_protocol.h"
#include "folder.h"
#include "timer.h"
#include "file_destination.h"
#include "utils.h"


//---Defines-------------------------------------------------------------------

#ifdef GLOBAL_INSTALLATION
    #define ZIX_CONFIG_FILENAME             "/usr/local/zix/zixconf.xml"
    #define ZIX_CONFIG_NEW_FILENAME	    "/usr/local/zix/zixconf_NEW.xml"
    #define ZIX_CONFIG_BACKUP_FILENAME      "/usr/local/zix/zixconf.xml.bak"
    #define ZIX_CONFIG_TEMP_FILENAME        "/usr/local/zix/zixconf.xml.tmp"
    #define ZIX_CONFIG_DAMAGED_FILENAME     "zixconf_damaged.xml"
    #define ZIX_CONFIG_DEFAULTS_FILENAME    "/usr/share/zix/defaults.xml"
#else
    #define ZIX_CONFIG_FILENAME             "zixconf.xml"
    #define ZIX_CONFIG_NEW_FILENAME	    "zixconf_NEW.xml"
    #define ZIX_CONFIG_BACKUP_FILENAME      "zixconf.xml.bak"
    #define ZIX_CONFIG_TEMP_FILENAME        "zixconf.xml.tmp"
    #define ZIX_CONFIG_DAMAGED_FILENAME     "zixconf_damaged.xml"
    #define ZIX_CONFIG_DEFAULTS_FILENAME    "defaults.xml"
#endif

#define ZIX_CONFIG_ROOT_NODE            "sic"


// When parameters are set for some handlers they only want to adopt the
// settings when the complete setConf command has finished.
// Therefore tthere are two signals;
// confChangeAnnounce(): This is emited on every parameter change. The handlers
// can use this signal and set a bit in the handlerMask when the config.
// confChanged(): This signal is emited at the end of a setConf command; the
// handlercan adopt the new configuration only when the bit was set before.
#define HANDLER_MASK_DU_MANAGER       0x0001
#define HANDLER_MASK_IPC_SERIAL       0x0002
#define HANDLER_MASK_USB_SERIAL       0x0004
#define HANDLER_MASK_COM1_SERIAL      0x0008
#define HANDLER_MASK_NETWORK          0x0010
#define HANDLER_MASK_SAMBA            0x0020
#define HANDLER_MASK_WEBSERVICE       0x0040
#define HANDLER_MASK_DEBUG_SERIAL     0x0080
#define HANDLER_MASK_NTPCONF          0x0100
#define HANDLER_MASK_LOG_HANDLER      0x0200
#define HANDLER_MASK_WATCHDOG_MANAGER 0x0400

//---Forward declaration-------------------------------------------------------


class XmlParameterList;


//---Declaration---------------------------------------------------------------


class ConfHandler: public Glib::Object
{
    private:
        static  Glib::RefPtr<ConfHandler> instance;
        //static  ConfHandler *instance;
        xmlpp::DomParser parser;
        xmlpp::Node* nodeRoot;
        //xmlpp::DomParser defaultsParser;
        //xmlpp::Node* defaultsNodeRoot;

        void initXmlDocument();

        Glib::ustring getValueOrValueRef(xmlpp::Element *elem);
        Glib::ustring language;
        int configChangedHandlerMask;

	void crypt_passwords ();
	void decrypt_passwords ();

	Glib::ustring convert_share(const Glib::ustring& value) const;
    public:
        ConfHandler();
        ~ConfHandler();

        static Glib::RefPtr<ConfHandler> get_instance();
        void clearConfigChangedHandlerMask()
        {
            configChangedHandlerMask=0;
        };
        static Glib::RefPtr<ConfHandler>create();
        xmlpp::Node *findCreateElement(
                xmlpp::Node *node, const Glib::ustring &xpath
                , const Glib::ustring &name, const Glib::ustring &id
                );
        xmlpp::Node *findCreateElement(
                xmlpp::Node *node, const Glib::ustring &xpath, int &changes );

        xmlpp::Element *findElement(xmlpp::Node *node, const Glib::ustring &path);
        xmlpp::Element *findElement( const Glib::ustring &path );

        std::string get_config_file_path() const noexcept;

        void load();
        void loadDefaults();
        void save();

        /// \brief  This method has to be called internally whenever a parameter was changed.
        ///
        ///         It saves the configuration and emits a signal.
        void parameterGotUpdated(   const Glib::ustring &parameterId
                           , const Glib::ustring &parameterValue );

        bool setConfParameter( const Glib::ustring& param , const Glib::ustring& unit,
                      const Glib::ustring& value );
        bool setConfNode( const Glib::ustring& node, const Glib::ustring& value, const Glib::ustring& unit);
        bool delConfNode (const Glib::ustring& node);
        bool getConf( const Glib::ustring& param
                     , Glib::ustring& unit, Glib::ustring& value );
        Glib::ustring getConf(const Glib::ustring& item, const Glib::ustring& id
                    , const Glib::ustring& resource, const Glib::ustring& postProcessor);
        Glib::ustring getConfParameter(const Glib::ustring& param);

        Glib::ustring getConfRegular(const Glib::ustring& item, const Glib::ustring& id
                                  , const Glib::ustring& resource, const Glib::ustring& postProcessor);

        Glib::ustring getConfFormatResource( const Glib::ustring& resource );

        Glib::ustring getConfNode( const Glib::ustring& node );

        Glib::ustring getConfTime();
        Glib::ustring getConfDate();

        /// \brief  return the current language
        ///
        ///         The language is not read dynamically from the configuration.
        ///         (reading from config might need current language). The
        ///         language is only set on start and on setConf
        Glib::ustring getLanguage()
        {
            return(language);
        }

        bool hoistName( xmlpp::Node *node );

        bool resetConf( const Glib::ustring & fname);
        sigc::signal<void, const Glib::ustring&, const Glib::ustring&, int &> confChangeAnnounce;
        sigc::signal<void, const int> confChanged;

    // snip -> internal config APIs <- snip
    Glib::ustring getDirectory( const Glib::ustring& id );

    Glib::ustring getAllocationProcessor(const Glib::ustring& id);

    std::vector<InputProcessor> getInputProcessorsByResource(const Glib::ustring& resource);

    InputProcessor getInputProcessorByType(const Glib::ustring& type);

    //bool dumpResource(const Glib::ustring& resource, Glib::ustring& text);
    Glib::ustring dumpResource(const Glib::ustring& resource);

    // <parameter id="foo" value="bar" />
    Glib::ustring getParameter(const Glib::ustring& id);
    Glib::ustring getPath(const Glib::ustring& id);

    // <parameter id="foo" value="enabled" />
    bool getBoolParameter (const Glib::ustring& id);

    // get log files
    std::vector<Glib::ustring> getAllLogFiles();

    // get monitor files
    std::vector<Glib::ustring> getAllMonitorFiles();

    // log buffer size
    std::size_t getLogBufferSize();

    // log level
    Glib::ustring getLogLevel();

    // serial protocol handler
    SerialProtocol getSerialProtocolHandler(const Glib::ustring& protocol_id);

    // lan protocol handler
    LanProtocol getLanProtocolHandler(const Glib::ustring& protocol_id);

    // get folder
    std::vector<Folder> getFolder(const FileDestination& dest, const Glib::ustring& type);
    std::vector<Folder> getInputFolder(const FileDestination& dest, const Glib::ustring& type);

    // get root folder, will be needed for usbFolder and lanFolder
    Glib::ustring getRootFolder(const FileDestination& dest);

    // get all postprocessors
    std::vector<PostProcessor> getAllPostProcessors();

    // get a specific post processor
    PostProcessor getSpecificPostProcessor(const FileDestination& dest, const Glib::ustring& format);

    //bool dumpResource(const Glib::ustring& resource, Glib::ustring& text);

    xmlpp::NodeSet getConfXmlByName(const Glib::ustring& item, xmlpp::Node* start=nullptr );

    // get a specific post processor
    PostProcessor getSpecificPostProcessor(const Glib::ustring& format);

    // get timer
    Timer getTimerByValueOrValueRef(xmlpp::Element *elem);

    Glib::ustring updatePrinters(const std::list<std::string> printers);
    void emitConfigChanged();
};


//---fin.----------------------------------------------------------------------
#endif // ! ? _CONF_HANDLER_H_
