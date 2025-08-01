import os

FILE_LIST_ENDPOINT = "/files"
RAW_ESP32_FILES_DIR = "raw_esp32_files"
GPX_FILES_DIR = "gpx_files"

class LocalStorageManager:
    def __init__(self):
        self.ensure_directories()

    def ensure_directories(self):
        if not os.path.exists(RAW_ESP32_FILES_DIR):
            os.makedirs(RAW_ESP32_FILES_DIR)
        if not os.path.exists(GPX_FILES_DIR):
            os.makedirs(GPX_FILES_DIR)

    def get_file_path_from_extension(self, file_name):
        if ".gpx" in file_name:
            return os.path.join(GPX_FILES_DIR, file_name)
        elif "csv" in file_name:
            return os.path.join(RAW_ESP32_FILES_DIR, file_name)
        
        return None

    def save_file_locally(self, file_name, file_content):

        if file_content is None:
            print(f"No content to save for {file_name}.")
            return None

        file_path = self.get_file_path_from_extension(file_name)
        if not file_path:
            print(f"Unsupported file type for {file_name}. Cannot save.")
            return None

        with open(file_path, 'wb') as file:
            if isinstance(file_content, str):
                file_content = file_content.encode('utf-8')
            file.write(file_content)

    def get_file_locally(self, file_name):
        try:
            with open(os.path.join(RAW_ESP32_FILES_DIR, file_name), 'rb') as file:
                return file.read()
        except FileNotFoundError:
            print(f"File {file_name} does not exist. Cannot read.")
            return None
        except Exception as e:
            print(f"Unexpected error while reading {file_name} , error: {e}")
        return None
    
    def delete_file_locally(self, file_name):
        try:
            os.remove(os.path.join(RAW_ESP32_FILES_DIR, file_name))
            print(f"File {file_name} deleted successfully.")
        except FileNotFoundError:
            print(f"File {file_name} does not exist. Cannot delete.")
        except Exception as e:
            print(f"Unexpected error while deleting {file_name}, error: {e}")
        return None
    
    def file_exists(self, file_name):
        return os.path.exists(os.path.join(RAW_ESP32_FILES_DIR, file_name))

if __name__ == "__main__":

    storage_manager = LocalStorageManager()

    content = storage_manager.get_file_locally("dog_run_173.csv")
    storage_manager.delete_file_locally("dog_run_173.csv")