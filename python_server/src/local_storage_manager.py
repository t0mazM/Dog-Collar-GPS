import os

RAW_ESP32_FILES_DIR = "raw_esp32_files"
GPX_FILES_DIR = "gpx_files"

class LocalStorageManager:
    def __init__(self):
        os.makedirs(RAW_ESP32_FILES_DIR, exist_ok=True)
        os.makedirs(GPX_FILES_DIR, exist_ok=True)

    def save_file_locally(self, file_name, file_content):
        with open(os.path.join(RAW_ESP32_FILES_DIR, file_name), 'wb') as file:
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