#include "SchemaWriter.hpp"

#include <utility>

namespace clp_structured {
void SchemaWriter::open(std::string path, int compression_level) {
    m_path = std::move(path);
    m_compression_level = compression_level;
}

void SchemaWriter::close() {
    m_compressor.close();
    m_file_writer.close();

    for (auto i : m_columns) {
        delete i;
    }

    m_columns.clear();
}

void SchemaWriter::append_column(BaseColumnWriter* column_writer) {
    m_columns.push_back(column_writer);
}

size_t SchemaWriter::append_message(ParsedMessage& message) {
    int count = 0;
    size_t size, total_size;
    size = total_size = 0;
    for (auto& i : message.get_content()) {
        m_columns[count]->add_value(i.second, size);
        total_size += size;
        count++;
    }

    m_num_messages++;
    return total_size;
}

void SchemaWriter::store() {
    m_file_writer.open(m_path, FileWriter::OpenMode::CreateForWriting);
    m_file_writer.write_numeric_value(m_num_messages);
    m_compressor.open(m_file_writer, m_compression_level);

    for (auto& writer : m_columns) {
        writer->store(m_compressor);
        //        compressor_.Write(writer->GetData(), writer->GetSize());
    }
}

SchemaWriter::~SchemaWriter() {
    for (auto i : m_columns) {
        delete i;
    }
}

void SchemaWriter::combine(SchemaWriter* writer) {
    for (size_t i = 0; i < m_columns.size(); ++i) {
        m_columns[i]->combine(writer->m_columns[i]);
    }
    m_num_messages += writer->m_num_messages;
    delete writer;
}

void SchemaWriter::update_schema(
        std::shared_ptr<SchemaTree> tree,
        std::vector<std::pair<int32_t, int32_t>> const& updates
) {
    std::vector<BaseColumnWriter*> new_columns;
    std::vector<TruncatedObjectColumnWriter*> new_truncated_columns;
    std::vector<BaseColumnWriter*> columns_to_delete;
    std::map<int32_t, BaseColumnWriter*> new_columns_map;
    for (BaseColumnWriter* writer : m_columns) {
        int32_t column_id = writer->get_id();
        int32_t new_column_id = -1;
        for (auto& update : updates) {
            if (update.first == column_id) {
                new_column_id = update.second;
                auto new_type = tree->get_node(update.second)->get_type();
                if (new_type == NodeType::TRUNCATEDOBJECT || new_type == NodeType::TRUNCATEDOBJECT)
                {
                    auto it = new_columns_map.find(update.second);
                    TruncatedObjectColumnWriter* new_writer = nullptr;
                    if (it == new_columns_map.end()) {
                        new_writer = new TruncatedObjectColumnWriter(
                                tree->get_node(new_column_id)->get_key_name(),
                                new_column_id
                        );
                        new_columns_map[new_column_id] = new_writer;
                        new_truncated_columns.push_back(new_writer);
                    } else {
                        new_writer = static_cast<TruncatedObjectColumnWriter*>(it->second);
                    }
                    new_writer->merge_column(writer, tree);
                }
                break;
            }
        }

        if (new_column_id != -1) {
            // for now all updates are cardinality one or truncation updates,
            // both of which remove the old column
            columns_to_delete.push_back(writer);
        } else {
            new_columns_map[column_id] = writer;
        }

        for (TruncatedObjectColumnWriter* writer : new_truncated_columns) {
            writer->local_merge_column_values(m_num_messages);
        }

        for (BaseColumnWriter* writer : columns_to_delete) {
            delete writer;
        }
    }

    for (auto const& pair : new_columns_map) {
        new_columns.push_back(pair.second);
    }

    m_columns = std::move(new_columns);
}

}  // namespace clp_structured
