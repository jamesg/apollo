#include "image_scaled.hpp"

#include <exiv2/exiv2.hpp>
#include <Magick++.h>

#include "atlas/http/server/mimetypes.hpp"
#include "hades/crud.ipp"
#include "hades/get_by_id.hpp"

#include "attachment.hpp"
#include "apollo/db.hpp"

void apollo::image_scaled(
        const atlas::http::mimetypes& mimetypes,
        hades::connection& conn,
        mg_connection *mg_conn,
        const int attachment_id,
        int width,
        int height,
        atlas::http::uri_callback_type callback_success,
        atlas::http::uri_callback_type callback_failure
        )
{
    try
    {
        attachment a = get_attachment(conn, attachment_info::id_type{attachment_id});

        const std::string filename(a.info.get_string<attr::attachment_orig_filename>());
        const std::string extension(filename.substr(filename.find_last_of(".") + 1));
        const std::string mimetype(mimetypes.content_type(extension));

        Magick::Blob blob;
        Magick::Image image(
            Magick::Blob(
                reinterpret_cast<const void*>(&(a.data[0])),
                a.data.size()
                )
            );
        short orientation = 1;
        try
        {
            // Retrieve orientation
            auto exiv_image = Exiv2::ImageFactory::open(
                reinterpret_cast<const unsigned char*>(
                    &(a.data[0])
                    ),
                a.data.size()
                );
            exiv_image->readMetadata();

            Exiv2::ExifKey key("Exif.Image.Orientation");
            Exiv2::ExifData::iterator pos = exiv_image->exifData().findKey(key);

            if(pos != exiv_image->exifData().end())
                orientation = pos->getValue()->toLong();
        }
        catch(const std::exception&)
        {
            // Some images don't have an orientation.
        }
        // Scale the image
        std::ostringstream oss;
        switch(orientation)
        {
        case 6:
        case 8:
            // Swap width and height because the image is about to be rotated
            std::swap(width, height);
        }
        // Scale to fit within this box
        oss << width << "x" << height << "^";
        image.scale(Magick::Geometry(oss.str()));

        // Rotate the image
        switch(orientation)
        {
            case 3:
                image.rotate(180);
            case 6:
                image.rotate(90);
            case 8:
                image.rotate(270);
        }

        Magick::Image out_image(image.size(), Magick::Color(255,255,255));
        out_image.composite(image, 0, 0);
        out_image.write(&blob, "JPEG");

        mg_send_status(mg_conn, 200);
        mg_send_header(mg_conn, "Content-type", "image/jpeg");
        mg_send_data(mg_conn, blob.data(), blob.length());
        //mg_send_data(
            //mg_conn,
            //&(a.get_string<attr::attachment_data>().c_str()[0]),
            //a.get_string<attr::attachment_data>().size()
            //);

        callback_success();
    }
    catch(const std::exception& e)
    {
        callback_failure();
    }
}

