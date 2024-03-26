#include "fecAlgorithm.hpp"

// Demonstration of how to use the C++ RAW interface
// it's pretty simple, we generate some input,
// then encode, drop some packets (source and repair)
// and finally decode everything.


// rename the main namespace for ease of use
namespace RaptorQ = RaptorQ__v1;


uint64_t Fec::getSymbolNum(uint64_t input_size, uint32_t symbol_size)
{
    auto min_symbols = (input_size * sizeof(uint8_t)) / symbol_size;
    if ((input_size * sizeof(uint8_t)) % symbol_size != 0)
        ++min_symbols;
    return min_symbols;
}

RaptorQ::Block_Size Fec::getBlockSize(uint64_t min_symbols)
{
    RaptorQ::Block_Size block = RaptorQ::Block_Size::Block_10;
    for (auto blk : *RaptorQ::blocks) {
        // RaptorQ::blocks is a pointer to an array, just scan it to find your
        // block.
        if (static_cast<uint16_t> (blk) >= min_symbols) {
            block = blk;
            break;
        }
    }
    return block;
}

std::vector<uint8_t> Fec::createVector(uint32_t chunk_id, uint32_t symbol_id, std::vector<uint8_t> data) {
    std::vector<uint8_t> result;

    // Copy the bytes of num1 and num2 into the vector
    result.resize(sizeof(uint32_t) * 2);
    std::memcpy(result.data(), &chunk_id, sizeof(uint32_t));
    std::memcpy(result.data() + sizeof(uint32_t), &symbol_id, sizeof(uint32_t));
    // insert the data info into the vector
    result.insert(result.end(), data.begin(), data.end());

    return result;
}

// *******************************************************************************************************
std::vector<std::pair<uint32_t, std::vector<uint8_t>>> Fec::encoder(
        const std::string& inputString, uint32_t symbol_size, uint32_t overhead)
{
    // the actual input.
    std::vector<uint8_t> input(inputString.begin(), inputString.end());
    // the input will be divided in blocks of 4 bytes.
    // it's a bit low, but this is just an example.
    // NOTE: the symbol size must be a multiple of the container size.
    //  since sizeof(uint8_t) == 1 and 4 is a multiple of 1, we are safe.

    //const uint16_t symbol_size = 4; // bytes //get this in parameter

    // how many symbols do we need to encode all our input in a single block?

    auto min_symbols = Fec::getSymbolNum(input.size(), symbol_size);
    // convert "symbols" to a typesafe equivalent, RaptorQ::Block_Size
    // This is needed because not all numbers are valid block sizes, and this
    // helps you choose the right block size
    RaptorQ::Block_Size block = Fec::getBlockSize(min_symbols);

    // now initialize the encoder.
    // the input for the encoder is std::vector<uint8_t>
    // the output for the encoder is std::vector<uint8_t>
    // yes, you can have different types, but most of the time you will
    // want to work with uint8_t
    RaptorQ::Encoder<typename std::vector<uint8_t>::iterator,
            typename std::vector<uint8_t>::iterator> enc (
            block, symbol_size);

    // give the input to the encoder. the encoder answers with the size of what
    // it can use
    if (enc.set_data (input.begin(), input.end()) != input.size()) {
        std::cout << "Could not give data to the encoder :(\n";
        //return false;
    }
    // actual symbols. you could just use static_cast<uint16_t> (block)
    // but this way you can actually query the encoder.
    uint16_t _symbols = enc.symbols();
    // print some stuff in output
    std::cout << "Size: " << input.size() << " symbols: " <<
              static_cast<uint32_t> (_symbols) <<
              " symbol size: " <<
              static_cast<int32_t>(enc.symbol_size()) << "\n";

    // RQ need to do its magic on the input before you can ask the symbols.
    // multiple ways to do this are available.
    // The simplest is to run the computation and block everything until
    // the work has been done. Not a problem for small sizes (<200),
    // but big sizes will take **a lot** of time, consider running this with the
    // asynchronous calls
    if (!enc.compute_sync()) {
        // if this happens it's a bug in the library.
        // the **Decoder** can fail, but the **Encoder** can never fail.
        std::cout << "Enc-RaptorQ failure! really bad!\n";
        throw std::runtime_error("Enc-RaptorQ failure! really bad!");
        //return false;
    }

    // we will store here all encoded and transmitted symbols
    // std::pair<symbol id (esi), symbol data>
    using symbol_id = uint32_t; // just a better name
    //std::vector<std::pair<symbol_id, std::vector<uint8_t>>> received;
    std::vector<std::pair<symbol_id, std::vector<uint8_t>>> received;
    {
        // in this block we will generate the symbols that will be sent to
        // the decoder.
        // a block of size X will need at least X symbols to be decoded.
        // we will randomly drop some symbols, but we will keep generating
        // repair symbols until we have the required number of symbols.

        // std::uniform_real_distribution<float> drop_rnd (0.0, 100.0);
        uint32_t received_tot = 0;

        // Now get the source symbols.
        // source symbols are specials because they contain the input data
        // as-is, so if you get all of these, you don't need repair symbols
        // to make sure that we are using the decoder, drop the first
        // source symbol.
        auto source_sym_it = enc.begin_source();
//        ++source_sym_it; // ignore the first source symbol (=> drop it)
//        source_sym_it++;
        for (; source_sym_it != enc.end_source(); ++source_sym_it) {
            // we save the symbol here:
            // make sure the vector has enough space for the symbol:
            // fill it with zeros for the size of the symbol
            std::vector<uint8_t> source_sym_data (symbol_size, 0);

            // save the data of the symbol into our vector
            auto it = source_sym_data.begin();
            auto written = (*source_sym_it) (it, source_sym_data.end());
            if (written != symbol_size) {
                // this can only happen if "source_sym_data" did not have
                // enough space for a symbol (here: never)
                std::cout << written << "-vs-" << symbol_size <<
                          " Could not get the whole source symbol!\n";
                //return false;
                throw std::runtime_error("Could not get the whole source symbol!");
            }

            // can we keep this symbol or do we randomly drop it?
//            float dropped = drop_rnd (rnd);
//            if (dropped <= drop_probability) {
//                continue; // start the cycle again
//            } // don't need to drop it

            // good, the symbol was received.
            ++received_tot;
            // add it to the vector of received symbols
            symbol_id tmp_id = (*source_sym_it).id();
            received.emplace_back (tmp_id, std::move(source_sym_data));
        }

        std::cout << "Source Packet lost: " << enc.symbols() - received.size()
                  << "\n";


        //--------------------------------------------
        // we finished working with the source symbols.
        // now we need to transmit the repair symbols.
        auto repair_sym_it = enc.begin_repair();
        auto max_repair = enc.max_repair(); // RaptorQ can theoretically handle
        // infinite repair symbols
        // but computers are not so infinite

        // we need to have at least enc.symbols() + overhead symbols.
        for (; received.size() < (enc.symbols() + overhead) &&
               repair_sym_it != enc.end_repair (max_repair);
               ++repair_sym_it) {
            // we save the symbol here:
            // make sure the vector has enough space for the symbol:
            // fill it with zeros for the size of the symbol
            std::vector<uint8_t> repair_sym_data (symbol_size, 0);

            // save the data of the symbol into our vector
            auto it = repair_sym_data.begin();
            auto written = (*repair_sym_it) (it, repair_sym_data.end());
            if (written != symbol_size) {
                // this can only happen if "repair_sym_data" did not have
                // enough space for a symbol (here: never)
                std::cout << written << "-vs-" << symbol_size <<
                          " Could not get the whole repair symbol!\n";
                //return false;
                throw std::runtime_error("Could not get the whole repair symbol!");
            }

            // can we keep this symbol or do we randomly drop it?
//            float dropped = drop_rnd (rnd);
//            if (dropped <= drop_probability) {
//                continue; // start the cycle again
//            } // don't need to drop

            // good, the symbol was received.
            ++received_tot;
            // add it to the vector of received symbols
            symbol_id tmp_id = (*repair_sym_it).id();
            received.emplace_back (tmp_id, std::move(repair_sym_data));
        }
        if (repair_sym_it == enc.end_repair (enc.max_repair())) {
            // we dropped waaaay too many symbols!
            // should never happen in real life. it means that we do not
            // have enough repair symbols.
            // at this point you can actually start to retransmit the
            // repair symbols from enc.begin_repair(), but we don't care in
            // this example
            //std::cout << "Maybe losing " << drop_probability << "% is too much?\n";
            //return false;
            throw std::runtime_error("we do not have enough repair symbols...");

        }
        //return true; //needs to return the received vector (contains all the encoded_symbols)
        return received;
    }
    // Now we all the source and repair symbols are in "received".
}

// end of encoder function
// *******************************************************************************************************





void printErrorMessage(RaptorQ::Error error) {
    switch (error) {
        case RaptorQ::Error::NONE:
            std::cout << "No error occurred." << std::endl;
            break;
        case RaptorQ::Error::NOT_NEEDED:
            std::cout << "Error: Not needed." << std::endl;
            break;
        case RaptorQ::Error::WRONG_INPUT:
            std::cout << "Error: Wrong input provided." << std::endl;
            break;
        case RaptorQ::Error::NEED_DATA:
            std::cout << "Error: Data needed." << std::endl;
            break;
        case RaptorQ::Error::WORKING:
            std::cout << "Error: Currently working." << std::endl;
            break;
        case RaptorQ::Error::INITIALIZATION:
            std::cout << "Error: Initialization failed." << std::endl;
            break;
        case RaptorQ::Error::EXITING:
            std::cout << "Error: Exiting." << std::endl;
            break;
        default:
            std::cout << "Unknown error code." << std::endl;
            break;
    }
}










//decoder function
std::vector<uint8_t> Fec::decoder(RaptorQ::Block_Size block, uint32_t chunk_size, uint32_t symbol_size,
        std::vector<std::pair<uint32_t, std::vector<uint8_t>>> received)
{
    // Now we all the source and repair symbols are in "received".
    // we will use those to start decoding:

    // the actual input.
    //std::vector<uint8_t> input(inputString.begin(), inputString.end());

    // do the same process at the encoder - just to get the block (write separate function to this purpose)
//    auto min_symbols = (input.size() * sizeof(uint8_t)) / symbol_size;
//    if ((input.size() * sizeof(uint8_t)) % symbol_size != 0)
//        ++min_symbols;
//    // convert "symbols" to a typesafe equivalent, RaptorQ::Block_Size
//    // This is needed because not all numbers are valid block sizes, and this
//    // helps you choose the right block size
//    RaptorQ::Block_Size block = RaptorQ::Block_Size::Block_10;
//    for (auto blk : *RaptorQ::blocks) {
//        // RaptorQ::blocks is a pointer to an array, just scan it to find your
//        // block.
//        if (static_cast<uint16_t> (blk) >= min_symbols) {
//            block = blk;
//            break;
//        }
//    }



    // define "Decoder_type" to write less afterwards
    using Decoder_type = RaptorQ::Decoder<
            typename std::vector<uint8_t>::iterator,
            typename std::vector<uint8_t>::iterator>;
    Decoder_type dec (block, symbol_size, Decoder_type::Report::COMPLETE);
    // "Decoder_type::Report::COMPLETE" means that the decoder will not
    // give us any output until we have decoded all the data.
    // there are modes to extract the data symbol by symbol in an ordered
    // an unordered fashion, but let's keep this simple.

    // we will store the output of the decoder here:
    // note: the output need to have at least "mysize" bytes, and
    // we fill it with zeros
    std::vector<uint8_t> output (chunk_size, 0);
    using symbol_id = uint32_t;
    std::cerr << "(decoder function) -> TOTAL SYMBOLS PACKETS TO DECODE: " << received.size() << std::endl;
    // now push every received symbol into the decoder
    for (auto &rec_sym : received) {
        // as a reminder:
        //  rec_sym.first = symbol_id (uint32_t)
        //  rec_sym.second = std::vector<uint8_t> symbol_data
        if (!rec_sym.second.empty()) // checking here if the symbol exist ...
        {
            std::cout << "adding symbol number: " << rec_sym.first << ", data: '" <<
                      std::string(rec_sym.second.begin(),rec_sym.second.end()) << "' to decoder..." << std::endl;
            symbol_id tmp_id = rec_sym.first;
            auto it = rec_sym.second.begin();
            auto err = dec.add_symbol(it, rec_sym.second.end(), tmp_id);
            if (err != RaptorQ::Error::NONE && err != RaptorQ::Error::NOT_NEEDED) {
                // When you add a symbol, you can get:
                //   NONE: no error
                //   NOT_NEEDED: libRaptorQ ignored it because everything is
                //              already decoded
                //   INITIALIZATION: wrong parameters to the decoder contructor
                //   WRONG_INPUT: not enough data on the symbol?
                //   some_other_error: errors in the library
                printErrorMessage(err);
                std::cout << "error adding?\n";
                //return false;
                throw std::runtime_error("error while adding symbols to decoder");
            }
        }
        else
        {
            std::cout << "From decoder -> the symbol at this place does not exist." << std::endl;
        }
    }

    // by now we now there will be no more input, so we tell this to the
    // decoder. You can skip this call, but if the decoder does not have
    // enough data it still waits forever (or until you call .stop())
    dec.end_of_input (RaptorQ::Fill_With_Zeros::NO);
    // optional if you want partial decoding without using the repair
    // symbols
    // std::vector<bool> symbols_bitmask = dec.end_of_input (
    //                                          RaptorQ::Fill_With_Zeros::YES);

    // decode, and do not return until the computation is finished.
    auto res = dec.wait_sync();
    if (res.error != RaptorQ::Error::NONE) {
        std::cout << "Couldn't decode.\n";
        //return false;
        throw std::runtime_error("Couldn't decode");
    }

    // now save the decoded data in our output
    size_t decode_from_byte = 0;
    size_t skip_bytes_at_begining_of_output = 0;
    auto out_it = output.begin();
    auto decoded = dec.decode_bytes(out_it, output.end(), decode_from_byte,
                                     skip_bytes_at_begining_of_output);
    // "decode_from_byte" can be used to have only a part of the output.
    // it can be used in advanced setups where you ask only a part
    // of the block at a time.
    // "skip_bytes_at_begining_of_output" is used when dealing with containers
    // which size does not align with the output. For really advanced usage only
    // Both should be zero for most setups.

    if (decoded.written != chunk_size) {
        if (decoded.written == 0) {
            // we were really unlucky and the RQ algorithm needed
            // more symbols!
            std::cout << "Couldn't decode, RaptorQ Algorithm failure. "
                         "Can't Retry.\n";
        } else {
            // probably a library error
            std::cout << "Partial Decoding? This should not have happened: " <<
                      decoded.written  << " vs " << chunk_size << "\n";
        }
        //return false;
        throw std::runtime_error("the RQ algorithm failure while decoding (needed more symbols or library error)");
    } else {
        std::cout << "Decoded: " << chunk_size << "\n";
    }

    // just for the example - checking if the original input is equal to the output string.
    // byte-wise check: did we actually decode everything the right way?
//    for (uint64_t i = 0; i < chunk_size; ++i) {
//        if (input[i] != output[i]) {
//            // this is a bug in the library, please report
//            std::cout << "The output does not correspond to the input!\n";
//            //return false;
//            throw std::runtime_error("The output does not correspond to the input!");
//        }
//    }


// needs to add a check here
// return from this function a vector of uint8_t and before writing the content to the file check:
// if the file is type of text-> convert this vector into string and write the content.
// if binary -> write the content.
    std::cout << "(from decoder) -> The output after decoding is:\n" << std::string(output.begin(), output.end()) << std::endl;
    //return true;
    return output; // returning a vector of uint_8 with the decoded_data
}










//
//// mysize is bytes. // instead mysize iet a serialized chunk that is part of the file.
//bool test_rq (const std::string& inputString, std::mt19937_64 &rnd,
//              float drop_probability,
//              const uint8_t overhead);
//// the "overhead" variable tells us how many symbols more than the
//// minimum we will generate. RaptorQ can not always decode a block,
//// but there is a small probability that it will fail.
//// More overhead => less probability of failure
////  overhead 0 => 1% failures
////  overhead 1 => 0.01% failures
////  overhead 2 => 0.0001% failures
//// etc... as you can see, it makes little sense to work with more than 3-4
//// overhead symbols, but at least one should be considered
//bool test_rq (const std::string& inputString, std::mt19937_64 &rnd,
//              float drop_probability,
//              const uint8_t overhead)
//{
//    // the actual input.
//    std::vector<uint8_t> input(inputString.begin(), inputString.end());
//
//    // the input will be divided in blocks of 4 bytes.
//    // it's a bit low, but this is just an example.
//    // NOTE: the symbol size must be a multiple of the container size.
//    //  since sizeof(uint8_t) == 1 and 4 is a multiple of 1, we are safe.
//    const uint16_t symbol_size = 4; // bytes
//
//
//    // how many symbols do we need to encode all our input in a single block?
//    auto min_symbols = (input.size() * sizeof(uint8_t)) / symbol_size;
//    if ((input.size() * sizeof(uint8_t)) % symbol_size != 0)
//        ++min_symbols;
//    // convert "symbols" to a typesafe equivalent, RaptorQ::Block_Size
//    // This is needed becouse not all numbers are valid block sizes, and this
//    // helps you choose the right block size
//    RaptorQ::Block_Size block = RaptorQ::Block_Size::Block_10;
//    for (auto blk : *RaptorQ::blocks) {
//        // RaptorQ::blocks is a pointer to an array, just scan it to find your
//        // block.
//        if (static_cast<uint16_t> (blk) >= min_symbols) {
//            block = blk;
//            break;
//        }
//    }
//
//
//    // now initialize the encoder.
//    // the input for the encoder is std::vector<uint8_t>
//    // the output for the encoder is std::vector<uint8_t>
//    // yes, you can have different types, but most of the time you will
//    // want to work with uint8_t
//    RaptorQ::Encoder<typename std::vector<uint8_t>::iterator,
//            typename std::vector<uint8_t>::iterator> enc (
//            block, symbol_size);
//
//    // give the input to the encoder. the encoder answers with the size of what
//    // it can use
//    if (enc.set_data (input.begin(), input.end()) != input.size()) {
//        std::cout << "Could not give data to the encoder :(\n";
//        return false;
//    }
//    // actual symbols. you could just use static_cast<uint16_t> (blok)
//    // but this way you can actually query the encoder.
//    uint16_t _symbols = enc.symbols();
//    // print some stuff in output
//    std::cout << "Size: " << input.size() << " symbols: " <<
//              static_cast<uint32_t> (_symbols) <<
//              " symbol size: " <<
//              static_cast<int32_t>(enc.symbol_size()) << "\n";
//
//    // RQ need to do its magic on the input before you can ask the symbols.
//    // multiple ways to do this are available.
//    // The simplest is to run the computation and block everything until
//    // the work has been done. Not a problem for small sizes (<200),
//    // but big sizes will take **a lot** of time, consider running this with the
//    // asynchronous calls
//    if (!enc.compute_sync()) {
//        // if this happens it's a bug in the library.
//        // the **Decoder** can fail, but the **Encoder** can never fail.
//        std::cout << "Enc-RaptorQ failure! really bad!\n";
//        return false;
//    }
//
//
//    // the probability that a symbol will be dropped.
//    if (drop_probability > static_cast<float> (90.0))
//        drop_probability = 90.0;   // this is still too high probably.
//
//
//    // we will store here all encoded and transmitted symbols
//    // std::pair<symbol id (esi), symbol data>
//    using symbol_id = uint32_t; // just a better name
//    std::vector<std::pair<symbol_id, std::vector<uint8_t>>> received;
//    {
//        // in this block we will generate the symbols that will be sent to
//        // the decoder.
//        // a block of size X will need at least X symbols to be decoded.
//        // we will randomly drop some symbols, but we will keep generating
//        // repair symbols until we have the required number of symbols.
//
//        std::uniform_real_distribution<float> drop_rnd (0.0, 100.0);
//        uint32_t received_tot = 0;
//
//        // Now get the source symbols.
//        // source symbols are specials because they contain the input data
//        // as-is, so if you get all of these, you don't need repair symbols
//        // to make sure that we are using the decoder, drop the first
//        // source symbol.
//        auto source_sym_it = enc.begin_source();
//        ++source_sym_it; // ignore the first soure symbol (=> drop it)
//        source_sym_it++;
//        for (; source_sym_it != enc.end_source(); ++source_sym_it) {
//            // we save the symbol here:
//            // make sure the vector has enough space for the symbol:
//            // fill it with zeros for the size of the symbol
//            std::vector<uint8_t> source_sym_data (symbol_size, 0);
//
//            // save the data of the symbol into our vector
//            auto it = source_sym_data.begin();
//            auto written = (*source_sym_it) (it, source_sym_data.end());
//            if (written != symbol_size) {
//                // this can only happen if "source_sym_data" did not have
//                // enough space for a symbol (here: never)
//                std::cout << written << "-vs-" << symbol_size <<
//                          " Could not get the whole source symbol!\n";
//                return false;
//            }
//
//            // can we keep this symbol or do we randomly drop it?
//            float dropped = drop_rnd (rnd);
//            if (dropped <= drop_probability) {
//                continue; // start the cycle again
//            }
//
//            // good, the symbol was received.
//            ++received_tot;
//            // add it to the vector of received symbols
//            symbol_id tmp_id = (*source_sym_it).id();
//            received.emplace_back (tmp_id, std::move(source_sym_data));
//        }
//
//        std::cout << "Source Packet lost: " << enc.symbols() - received.size()
//                  << "\n";
//
//
//        //--------------------------------------------
//        // we finished working with the source symbols.
//        // now we need to transmit the repair symbols.
//        auto repair_sym_it = enc.begin_repair();
//        auto max_repair = enc.max_repair(); // RaptorQ can theoretically handle
//        // infinite repair symbols
//        // but computers are not so infinite
//
//        // we need to have at least enc.symbols() + overhead symbols.
//        for (; received.size() < (enc.symbols() + overhead) &&
//               repair_sym_it != enc.end_repair (max_repair);
//               ++repair_sym_it) {
//            // we save the symbol here:
//            // make sure the vector has enough space for the symbol:
//            // fill it with zeros for the size of the symbol
//            std::vector<uint8_t> repair_sym_data (symbol_size, 0);
//
//            // save the data of the symbol into our vector
//            auto it = repair_sym_data.begin();
//            auto written = (*repair_sym_it) (it, repair_sym_data.end());
//            if (written != symbol_size) {
//                // this can only happen if "repair_sym_data" did not have
//                // enough space for a symbol (here: never)
//                std::cout << written << "-vs-" << symbol_size <<
//                          " Could not get the whole repair symbol!\n";
//                return false;
//            }
//
//            // can we keep this symbol or do we randomly drop it?
//            float dropped = drop_rnd (rnd);
//            if (dropped <= drop_probability) {
//                continue; // start the cycle again
//            }
//
//            // good, the symbol was received.
//            ++received_tot;
//            // add it to the vector of received symbols
//            symbol_id tmp_id = (*repair_sym_it).id();
//            received.emplace_back (tmp_id, std::move(repair_sym_data));
//
//        }
//        if (repair_sym_it == enc.end_repair (enc.max_repair())) {
//            // we dropped waaaay too many symbols!
//            // should never happen in real life. it means that we do not
//            // have enough repair symbols.
//            // at this point you can actually start to retransmit the
//            // repair symbols from enc.begin_repair(), but we don't care in
//            // this example
//            std::cout << "Maybe losing " << drop_probability << "% is too much?\n";
//            return false;
//        }
//    }
//
//    // Now we all the source and repair symbols are in "received".
//    // we will use those to start decoding:
//
//
//    // define "Decoder_type" to write less afterwards
//    using Decoder_type = RaptorQ::Decoder<
//            typename std::vector<uint8_t>::iterator,
//            typename std::vector<uint8_t>::iterator>;
//    Decoder_type dec (block, symbol_size, Decoder_type::Report::COMPLETE);
//    // "Decoder_type::Report::COMPLETE" means that the decoder will not
//    // give us any output until we have decoded all the data.
//    // there are modes to extract the data symbol by symbol in an ordered
//    // an unordered fashion, but let's keep this simple.
//
//
//    // we will store the output of the decoder here:
//    // note: the output need to have at least "mysize" bytes, and
//    // we fill it with zeros
//    std::vector<uint8_t> output (input.size(), 0);
//
//    // now push every received symbol into the decoder
//    for (auto &rec_sym : received) {
//        // as a reminder:
//        //  rec_sym.first = symbol_id (uint32_t)
//        //  rec_sym.second = std::vector<uint8_t> symbol_data
//        symbol_id tmp_id = rec_sym.first;
//        auto it = rec_sym.second.begin();
//        auto err = dec.add_symbol (it, rec_sym.second.end(), tmp_id);
//        if (err != RaptorQ::Error::NONE && err != RaptorQ::Error::NOT_NEEDED) {
//            // When you add a symbol, you can get:
//            //   NONE: no error
//            //   NOT_NEEDED: libRaptorQ ignored it because everything is
//            //              already decoded
//            //   INITIALIZATION: wrong parameters to the decoder contructor
//            //   WRONG_INPUT: not enough data on the symbol?
//            //   some_other_error: errors in the library
//            std::cout << "error adding?\n";
//            return false;
//        }
//    }
//
//    // by now we now there will be no more input, so we tell this to the
//    // decoder. You can skip this call, but if the decoder does not have
//    // enough data it still waits forever (or until you call .stop())
//    dec.end_of_input (RaptorQ::Fill_With_Zeros::NO);
//    // optional if you want partial decoding without using the repair
//    // symbols
//    // std::vector<bool> symbols_bitmask = dec.end_of_input (
//    //                                          RaptorQ::Fill_With_Zeros::YES);
//
//    // decode, and do not return until the computation is finished.
//    auto res = dec.wait_sync();
//    if (res.error != RaptorQ::Error::NONE) {
//        std::cout << "Couldn't decode.\n";
//        return false;
//    }
//
//    // now save the decoded data in our output
//    size_t decode_from_byte = 0;
//    size_t skip_bytes_at_begining_of_output = 0;
//    auto out_it = output.begin();
//    auto decoded = dec.decode_bytes (out_it, output.end(), decode_from_byte,
//                                     skip_bytes_at_begining_of_output);
//    // "decode_from_byte" can be used to have only a part of the output.
//    // it can be used in advanced setups where you ask only a part
//    // of the block at a time.
//    // "skip_bytes_at_begining_of_output" is used when dealing with containers
//    // which size does not align with the output. For really advanced usage only
//    // Both should be zero for most setups.
//
//    if (decoded.written != input.size()) {
//        if (decoded.written == 0) {
//            // we were really unlucky and the RQ algorithm needed
//            // more symbols!
//            std::cout << "Couldn't decode, RaptorQ Algorithm failure. "
//                         "Can't Retry.\n";
//        } else {
//            // probably a library error
//            std::cout << "Partial Decoding? This should not have happened: " <<
//                      decoded.written  << " vs " << input.size() << "\n";
//        }
//        return false;
//    } else {
//        std::cout << "Decoded: " << input.size() << "\n";
//    }
//
//    // byte-wise check: did we actually decode everything the right way?
//    for (uint64_t i = 0; i < input.size(); ++i) {
//        if (input[i] != output[i]) {
//            // this is a bug in the library, please report
//            std::cout << "The output does not correspond to the input!\n";
//            return false;
//        }
//    }
//    return true;
//}

//int main() {
//    // Initialize random number generator
//    std::random_device rd;
//    std::mt19937_64 rnd(rd());
//
//    // Example protobuf structure
//    FILE_STORAGE::ConfigPacket configPacket;
//    configPacket.set_type(FILE_STORAGE::FileType::DIRECTORY);
//    configPacket.set_chunks(100);
//    configPacket.set_chunk_size(20);
//    configPacket.set_name("successful encode decode! aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
//
//
//    // serialized the data structure into string
//    //std::string inputString = configPacket.SerializeAsString();
//
//    std::string inputString = "hello world! im a sentence... 12345678932165498798765431123456789";
//
//    // std::string inputString = "Hello, world!";
//    std::cout << "The size of the serialized data is: " << inputString.size() << std::endl;
//    // Drop probability and overhead
//    float drop_probability = 10.0; // Example value
//    uint8_t overhead = 5; // Example value
//
//    // Call test_rq function
//    if (test_rq(inputString, rnd, drop_probability, overhead)) {
//        std::cout << "Test succeeded!\n";
//    } else {
//        std::cout << "Test failed!\n";
//    }
//    uint32_t symbol_size = 5;
//
//    //std::vector<std::pair<uint32_t, std::vector<uint8_t>>> encoder(const std::string& inputString, uint32_t symbol_size, const uint8_t overhead)
//    //bool decoder(const std::string& inputString, uint32_t symbol_size, std::vector<std::pair<uint32_t, std::vector<uint8_t>>> received)
//    try {
//        std::string output = fec::decoder(inputString, symbol_size, fec::encoder(inputString, symbol_size, overhead));
//        std::cout << "Test (encoder + decoder function) succeeded!\n";
//        std::cout << "The  encoded data is: " << output << std::endl;
//    } catch (std::exception& e) {
//        std::cout << "Test (encoder + decoder function) failed!\n";
//        std::cout << "error occurred... -> " << e.what();
//    }
//    return 0;
//}
