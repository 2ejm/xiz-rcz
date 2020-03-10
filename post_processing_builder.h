#ifndef _POST_PROCESSING_BUILDER_H_
#define _POST_PROCESSING_BUILDER_H_

#include <glibmm/ustring.h>

#include <string>
#include <vector>

#include "format.h"
#include "post_processor.h"
#include "post_processing_entry.h"
#include "file_destination.h"

/**
 * \brief Encapsulates functionality in order to setup a post processing queue.
 *
 * This is common functionality used by dataOut and getMeasurement.
 */
class PostProcessingBuilder
{
public:
    using PostProcessingQueue = std::vector<PostProcessingEntry>;

    PostProcessingBuilder()
    {}

    /**
     * Gets a post processing queue specified by id/iid and format.
     *
     * Used by getMeasurement.
     *
     * @param iid    measurement iid
     * @param format format e.g. PDF standard
     *
     * @return pp queue
     */
    PostProcessingQueue build_queue_without_dest(const Glib::ustring& iid, const Glib::ustring& format);

    /**
     * Creates a post processing queue based on iid and destination
     * parameter. format and output can be omitted. Then all available
     * formats/output channels are used as specified in the zixconf.xml.
     *
     * Used by dataOut.
     *
     * @param iid    measurement iid
     * @param dest   destination e.g. lanPrinter
     * @param format format e.g. PDF standard    [optional]
     * @param output output channel e.g. 2       [optional]
     *
     * @return postprocessing queue
     */
    PostProcessingQueue build_queue(const Glib::ustring& iid, const FileDestination& dest,
                                    const Glib::ustring& format = "", const Glib::ustring& output = "");

private:
    std::vector<std::string>
    generated_files_by_pp(const PostProcessor& pp, const Glib::ustring& iid,
                          const Glib::ustring& format_name) const;

    std::string get_input_extension(const Glib::ustring& code) const;

    PostProcessingQueue build_queue_file_dest(const Glib::ustring& iid, const FileDestination& dest,
                                              const Glib::ustring& format = "", const Glib::ustring& output = "");

    PostProcessingQueue build_queue_other_dest(const Glib::ustring& iid, const FileDestination& dest,
                                               const Glib::ustring& format = "");
};

#endif /* _POST_PROCESSING_BUILDER_H_ */
