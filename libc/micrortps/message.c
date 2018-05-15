
// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "message.h"
#include "xrce_protocol_serialization.h"
#include <string.h>

#define YELLOW_DARK    "\x1B[0;33m"
#define PURPLE_DARK    "\x1B[0;35m"
#define GREY_LIGHT     "\x1B[0;37m"

#define GREY_DARK      "\x1B[1;30m"
#define RED            "\x1B[1;31m"
#define GREEN          "\x1B[1;32m"
#define YELLOW         "\x1B[1;33m"
#define BLUE           "\x1B[1;34m"
#define PURPLE         "\x1B[1;35m"
#define CYAN           "\x1B[1;36m"
#define WHITE          "\x1B[1;37m"

#define RESTORE_COLOR  "\x1B[0m"

#define SEND_ARROW "==> "
#define RECV_ARROW "<== "

static const char* data_to_string(const uint8_t* data, uint32_t size)
{
  uint32_t i;
    static char buffer[4096];
    for(i = 0; i < size; i++)
        sprintf(buffer + 3 * i, "%02X ", data[i]);
    return buffer;
}

static const char* request_to_string(const BaseObjectRequest* request)
{
    static char buffer[256];
    sprintf(buffer, "#0x%04X | id: 0x%04X",
            get_num_request_id(request->request_id),
            get_num_object_id(request->object_id));

    return buffer;
}

static const char* reply_to_string(const BaseObjectReply* reply)
{
    static char buffer[256];

    char status[64];
    switch(reply->result.status)
    {
        case STATUS_OK:
            sprintf(status, "OK");
            break;
        case STATUS_OK_MATCHED:
            sprintf(status, "OK_MATCHED");
            break;
        case STATUS_ERR_DDS_ERROR:
            sprintf(status, "ERR_DDS_ERROR");
            break;
        case STATUS_ERR_MISMATCH:
            sprintf(status, "ERR_MISMATCH");
            break;
        case STATUS_ERR_ALREADY_EXISTS:
            sprintf(status, "ERR_ALREADY_EXISTS");
            break;
        case STATUS_ERR_DENIED:
            sprintf(status, "ERR_DENIED");
            break;
        case STATUS_ERR_UNKNOWN_REFERENCE:
            sprintf(status, "ERR_UNKNOWN_REFERENCE");
            break;
        case STATUS_ERR_INVALID_DATA:
            sprintf(status, "ERR_INVALID_DATA");
            break;
        case STATUS_ERR_INCOMPATIBLE:
            sprintf(status, "ERR_INCOMPATIBLE");
            break;
        case STATUS_ERR_RESOURCES:
            sprintf(status, "ERR_RESOURCES");
            break;
        default:
            sprintf(status, "UNKNOWN");
    }

    sprintf(buffer, "#0x%04X | id: 0x%04X | from #0x%04X | %s",
            get_num_request_id(reply->related_request.request_id),
            get_num_object_id(reply->related_request.object_id),
            get_num_request_id(reply->related_request.request_id),
            status);

    return buffer;
}

static void print_create_client_submessage(const char* pre, const CREATE_CLIENT_Payload* payload)
{
    printf("%s[Create client | %s | session: 0x%02X | key: %s]%s\n",
            pre,
            request_to_string(&payload->base),
            payload->client_representation.session_id,
            data_to_string(payload->client_representation.client_key.data, 4),
            RESTORE_COLOR);
}

static void print_create_submessage(const char* pre, const CREATE_Payload* payload)
{
    char content[4096];
    switch(payload->object_representation.kind)
    {
        case OBJK_PARTICIPANT:
            sprintf(content, "PARTICIPANT");
            break;
        case OBJK_TOPIC:
            sprintf(content, "TOPIC | id: 0x%04X | topic: %u",
                    get_num_object_id(payload->object_representation._.data_reader.subscriber_id),
                    payload->object_representation._.data_reader.base.representation._.string_represenatation.size);
            break;
        case OBJK_PUBLISHER:
            sprintf(content, "PUBLISHER | id: 0x%04X",
                    get_num_object_id(payload->object_representation._.publisher.participant_id));
            break;
        case OBJK_SUBSCRIBER:
            sprintf(content, "SUBSCRIBER | id: 0x%04X",
                    get_num_object_id(payload->object_representation._.subscriber.participant_id));
            break;
        case OBJK_DATAWRITER:
            sprintf(content, "DATA_WRITER | id: 0x%04X | xml: %u",
                    get_num_object_id(payload->object_representation._.data_writer.publisher_id),
                    payload->object_representation._.data_writer.base.representation._.string_represenatation.size);
             break;
        case OBJK_DATAREADER:
            sprintf(content, "DATA_READER | id: 0x%04X | xml: %u",
                    get_num_object_id(payload->object_representation._.data_reader.subscriber_id),
                    payload->object_representation._.data_reader.base.representation._.string_represenatation.size);
            break;
        default:
            sprintf(content, "UNKNOWN");
    }

    printf("%s[Create | %s | %s]%s\n",
            pre,
            request_to_string(&payload->base),
            content,
            RESTORE_COLOR);
}

/*static void print_get_info_submessage(const char* pre, const GET_INFO_Payload* payload)
{
    //TODO
    (void) payload;
}*/

static void print_delete_submessage(const char* pre, const DELETE_Payload* payload)
{
    printf("%s[Delete | %s]%s\n",
            pre,
            request_to_string(&payload->base),
            RESTORE_COLOR);
}

static void print_status_submessage(const char* pre, const STATUS_Payload* payload)
{
    printf("%s[Status | %s]%s\n",
            pre,
            reply_to_string(&payload->base),
            RESTORE_COLOR);
}

/*static void print_info_submessage(const char* pre, const INFO_Payload* payload)
{
    //TODO
    (void) payload;
}*/

static void print_read_data_submessage(const char* pre, const READ_DATA_Payload* payload)
{
    char format[128];
    switch(payload->read_specification.data_format)
    {
        case FORMAT_DATA:
            sprintf(format, "FORMAT_DATA");
        break;
        case FORMAT_DATA_SEQ:
            sprintf(format, "FORMAT_DATA_SEQ");
        break;
        case FORMAT_SAMPLE:
            sprintf(format, "FORMAT_SAMPLE");
        break;
        case FORMAT_SAMPLE_SEQ:
            sprintf(format, "FORMAT_SAMPLE_SEQ");
        break;
        case FORMAT_PACKED_SAMPLES:
            sprintf(format, "FORMAT_PACKED_SAMPLES");
        break;
        default:
            sprintf(format, "FORMAT_DATA_UNKNOWN");
    }

    printf("%s[Read data | %s | %s]%s\n",
            pre,
            request_to_string(&payload->base),
            format,
            RESTORE_COLOR);
}

static void print_write_data_data_submessage(const char* pre, const WRITE_DATA_Payload_Data* payload)
{
    printf("%s[Write data | FORMAT_DATA | %s]%s\n",
            pre,
            request_to_string(&payload->base),
            RESTORE_COLOR);
}

static void print_data_data_submessage(const char* pre, const DATA_Payload_Data* payload)
{
    printf("%s[Data | FORMAT_DATA | %s]%s\n",
            pre,
            request_to_string(&payload->base),
            RESTORE_COLOR);
}

static void print_acknack_submessage(const char* pre, const ACKNACK_Payload* payload)
{
  int i;
    char bitmask[17] = {0};
    for(i = 0; i < 8; i++)
    {
        bitmask[15 - i] = (payload->nack_bitmap[1] & (1 << i)) ? '1' : '0';
        bitmask[7 - i] = (payload->nack_bitmap[0] & (1 << i)) ? '1' : '0';
    }

    printf("%s[Acknack | seq_num: %hu | bitmap: %s]%s\n",
            pre,
            payload->first_unacked_seq_num,
            bitmask,
            RESTORE_COLOR);
}

static void print_heartbeat_submessage(const char* pre, const HEARTBEAT_Payload* payload)
{
    printf("%s[Heartbeat | first: %hu | last: %hu]%s\n",
            pre,
            payload->first_unacked_seq_nr,
            payload->last_unacked_seq_nr,
            RESTORE_COLOR);
}

void print_header(const char* dir, const MessageHeader* header, bool printable)
{

    if(printable)
    {
        char stream_representation;
        switch(header->stream_id)
        {
            case STREAMID_NONE:
                stream_representation = 'N';
                break;
            case STREAMID_BUILTIN_BEST_EFFORTS:
                stream_representation = 'B';
                break;
            case STREAMID_BUILTIN_RELIABLE:
                stream_representation = 'R';
                break;
            default:
                stream_representation = '-';
        }

        uint16_t seq_num = 0;
        if(header->stream_id != 0)
        {
            seq_num = header->sequence_nr;
        }

        printf("%s%s[%c | %hu]%s", dir, GREY_LIGHT, stream_representation, seq_num, RESTORE_COLOR);
    }
    else
    {
        printf("           ");
    }
}

void print_message(int direction, uint8_t* buffer, uint32_t size)
{
    const char* dir = (direction == SEND) ? SEND_ARROW : RECV_ARROW;
    const char* color = (direction == SEND) ? YELLOW : PURPLE;

    MicroBuffer micro_buffer;
    init_micro_buffer(&micro_buffer, buffer, size);

    MessageHeader header;
    deserialize_MessageHeader(&micro_buffer, &header);
    if (128 > header.session_id)
    {
        ClientKey key;
        deserialize_ClientKey(&micro_buffer, &key);
    }

    //bool is_reliable = header.stream_id

    bool first_submessage = true;
    while(micro_buffer.final - micro_buffer.iterator > SUBHEADER_SIZE)
    {
        SubmessageHeader sub_header;
        deserialize_SubmessageHeader(&micro_buffer, &sub_header);
        micro_buffer.endianness = (sub_header.flags & 0x01) ? LITTLE_ENDIANNESS : BIG_ENDIANNESS;

        print_header(dir, &header, first_submessage);

        if(sub_header.length > micro_buffer.final - micro_buffer.iterator)
        {
            printf("%s[INCOMPLETE SUBMESSAGE]%s\n", RED, RESTORE_COLOR);
            break;
        }

        switch(sub_header.id)
        {
            case SUBMESSAGE_ID_CREATE_CLIENT:
            {
                CREATE_CLIENT_Payload payload;
                deserialize_CREATE_CLIENT_Payload(&micro_buffer, &payload);
                print_create_client_submessage(color, &payload);

            } break;

            case SUBMESSAGE_ID_CREATE:
            {
                CREATE_Payload payload;
                deserialize_CREATE_Payload(&micro_buffer, &payload);
                print_create_submessage(color, &payload);

            } break;

            case SUBMESSAGE_ID_GET_INFO:
            {
                printf("%s[GET INFO SUBMESSAGE (not supported)]%s\n", RED, RESTORE_COLOR);

            } break;

            case SUBMESSAGE_ID_DELETE:
            {
                DELETE_Payload payload;
                deserialize_DELETE_Payload(&micro_buffer, &payload);
                print_delete_submessage(color, &payload);

            } break;

            case SUBMESSAGE_ID_STATUS:
            {
                STATUS_Payload payload;
                deserialize_STATUS_Payload(&micro_buffer, &payload);
                print_status_submessage(color, &payload);

            } break;

            case SUBMESSAGE_ID_INFO:
            {
                printf("%s[INFO SUBMESSAGE (not supported)]%s\n", RED, RESTORE_COLOR);
            } break;

            case SUBMESSAGE_ID_WRITE_DATA:
            {
                WRITE_DATA_Payload_Data payload;
                deserialize_WRITE_DATA_Payload_Data(&micro_buffer, &payload);
                print_write_data_data_submessage(color, &payload);

            } break;

            case SUBMESSAGE_ID_READ_DATA:
            {
                READ_DATA_Payload payload;
                deserialize_READ_DATA_Payload(&micro_buffer, &payload);
                print_read_data_submessage(color, &payload);

            } break;

            case SUBMESSAGE_ID_HEARTBEAT:
            {
                HEARTBEAT_Payload payload;
                deserialize_HEARTBEAT_Payload(&micro_buffer, &payload);
                print_heartbeat_submessage(color, &payload);

            } break;

            case SUBMESSAGE_ID_ACKNACK:
            {
                ACKNACK_Payload payload;
                deserialize_ACKNACK_Payload(&micro_buffer, &payload);
                print_acknack_submessage(color, &payload);

            } break;

            case SUBMESSAGE_ID_DATA:
            {
                DATA_Payload_Data payload;
                deserialize_DATA_Payload_Data(&micro_buffer, &payload);
                print_data_data_submessage(color, &payload);

            } break;

            case SUBMESSAGE_ID_FRAGMENT:
            {
                printf("%s[FRAGMENT SUBMESSAGE (not supported)]%s\n", RED, RESTORE_COLOR);
            } break;

            case SUBMESSAGE_ID_FRAGMENT_END:
            {
                printf("%s[FRAGMENT END SUBMESSAGE (not supported)]%s\n", RED, RESTORE_COLOR);
            } break;

            default:
            {
                printf("%s[UNKNOWN SUBMESSAGE]%s\n", RED, RESTORE_COLOR);
            } break;
        }

        align_to(&micro_buffer, 4);
        first_submessage = false;
    }
}

void print_serialization(int direction, const uint8_t* buffer, uint32_t size)
{
    const char* dir = (direction == SEND) ? SEND_ARROW : RECV_ARROW;

    printf("%s%s<< [%u]: %s>>%s\n",
            dir,
            GREY_DARK,
            size,
            data_to_string(buffer, size),
            RESTORE_COLOR);
}
