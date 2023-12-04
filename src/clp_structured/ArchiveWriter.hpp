#ifndef CLP_STRUCTURED_ARCHIVEWRITER_HPP
#define CLP_STRUCTURED_ARCHIVEWRITER_HPP

#include <set>
#include <utility>

#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "DictionaryWriter.hpp"
#include "SchemaMap.hpp"
#include "SchemaTree.hpp"
#include "SchemaWriter.hpp"
#include "TimestampDictionaryWriter.hpp"

namespace clp_structured {
struct ArchiveWriterOption {
    boost::uuids::uuid id;
    std::string archive_dir;
    int compression_level;
};

class ArchiveWriter {
public:
    class OperationFailed : public TraceableException {
    public:
        // Constructors
        OperationFailed(ErrorCode error_code, char const* const filename, int line_number)
                : TraceableException(error_code, filename, line_number) {}
    };

    // Delete default constructor
    ArchiveWriter() = delete;

    // Constructor
    explicit ArchiveWriter(
            std::shared_ptr<SchemaTree> schema_tree,
            std::shared_ptr<TimestampDictionaryWriter> timestamp_dict,
            std::shared_ptr<SchemaMap>& schema_map
    )
            : m_encoded_message_size(0UL),
              m_num_encoded_messages(0UL),
              m_schema_tree(std::move(schema_tree)),
              m_timestamp_dict(std::move(timestamp_dict)),
              m_schema_map(schema_map) {}

    /**
     * Opens the archive writer
     * @param option
     */
    void open(ArchiveWriterOption const& option);

    /**
     * Closes the archive writer
     */
    void close();

    /**
     * Appends a message to the archive writer
     * @param schema_id
     * @param schema
     * @param message
     */
    void append_message(int32_t schema_id, std::set<int32_t>& schema, ParsedMessage& message);

    /**
     * @return Size of the uncompressed data written to the archive
     */
    size_t get_data_size();

    /**
     * @return Number of records encoded in the archive
     */
    size_t get_num_records() { return m_num_encoded_messages; }

private:
    /**
     * Initializes the schema writer
     * @param writer
     * @param schema
     */
    void initialize_schema_writer(SchemaWriter* writer, std::set<int32_t>& schema);

    size_t m_encoded_message_size;
    size_t m_num_encoded_messages;

    boost::uuids::uuid m_id{};

    std::string m_archive_path;
    std::string m_encoded_messages_dir;

    std::shared_ptr<VariableDictionaryWriter> m_var_dict;
    std::shared_ptr<LogTypeDictionaryWriter> m_log_dict;
    std::shared_ptr<LogTypeDictionaryWriter> m_array_dict;  // log type dictionary for arrays
    std::shared_ptr<TimestampDictionaryWriter> m_timestamp_dict;
    int m_compression_level{};

    std::shared_ptr<SchemaTree> m_schema_tree;
    std::map<int32_t, SchemaWriter*> m_schema_id_to_writer;
    std::shared_ptr<SchemaMap> m_schema_map;
};
}  // namespace clp_structured

#endif  // CLP_STRUCTURED_ARCHIVEWRITER_HPP
