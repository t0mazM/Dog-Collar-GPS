from dogcollar_client import DogCollarClient
from local_storage_manager import GPX_FILES_DIR
import os


if __name__ == "__main__":
        client = DogCollarClient("http://dogcollar.local")

        # 1) Get the list of files 
        while True:
            file_names = client.get_file_list()

            for file_name in file_names:

                # 2) Download each file 
                if not client.download_file(file_name):
                    continue # If file was already downloaded, we don't do any operations

                # 3) Open file
                file = client.storage_manager.get_file_locally(file_name)

                # 4) Convert to gpx
                gpx_file = client.GPXConverter.convert_to_gpx(file, file_name)

                # 5) Save gpx file locally
                gpx_file_name = file_name.replace('.csv', '.gpx')
                client.storage_manager.save_file_locally(gpx_file_name, gpx_file)

                # 6) Upload to Strava
                client.strava_uploader.upload_gpx_file(os.path.join(GPX_FILES_DIR, gpx_file_name))


    