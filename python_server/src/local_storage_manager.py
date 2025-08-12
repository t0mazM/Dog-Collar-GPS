from logging_util import get_logger
import os

FILE_LIST_ENDPOINT = "/files"
DOWNLOAD_FILE_ENDPOINT = "download?file="
RAW_ESP32_FILES_DIR = "raw_esp32_files"
GPX_FILES_DIR = "gpx_files"

logger = get_logger(__name__)


class LocalStorageManager:
    def __init__(self):
        self.ensure_directories() # Make sure directories exist

    def ensure_directories(self) -> None:
        if not os.path.exists(RAW_ESP32_FILES_DIR):
            os.makedirs(RAW_ESP32_FILES_DIR)
        if not os.path.exists(GPX_FILES_DIR):
            os.makedirs(GPX_FILES_DIR)

    # Get the file path for storing the file based on the file extension
    def get_file_path_from_extension(self, file_name: str) -> str | None:
        if ".gpx" in file_name:
            return os.path.join(GPX_FILES_DIR, file_name)
        elif "csv" in file_name:
            return os.path.join(RAW_ESP32_FILES_DIR, file_name)
        
        return None

    def save_file_locally(self, file_name: str, file_content: bytes | str | None) -> None:

        # If no content is provided, do not save
        if file_content is None:
            logger.error(f"No content to save for {file_name}.")
            return None

        # Get the file path for saving
        file_path = self.get_file_path_from_extension(file_name)
        if not file_path:
            logger.error(f"Unsupported file type for {file_name}. Cannot save.")
            return None

        # Save the file content
        with open(file_path, 'wb') as file:
            if isinstance(file_content, str):
                file_content = file_content.encode('utf-8')
            file.write(file_content)

    def get_file_locally(self, file_name: str) -> bytes | None:
        try:
            with open(os.path.join(RAW_ESP32_FILES_DIR, file_name), 'rb') as file:
                return file.read()
        except FileNotFoundError:
            logger.error(f"File {file_name} does not exist. Cannot read.")
            return None
        except Exception as e:
            logger.error(f"Unexpected error while reading {file_name} , error: {e}")
        return None

    def delete_file_locally(self, file_name: str) -> None:
        try:
            os.remove(os.path.join(RAW_ESP32_FILES_DIR, file_name))
            logger.info(f"File {file_name} deleted successfully.")
        except FileNotFoundError:
            logger.error(f"File {file_name} does not exist. Cannot delete.")
        except Exception as e:
            logger.error(f"Unexpected error while deleting {file_name}, error: {e}")
        return None

    def file_exists(self, file_name: str) -> bool:
        return os.path.exists(os.path.join(RAW_ESP32_FILES_DIR, file_name))

# Example usage
if __name__ == "__main__":

    TEST_FILE_NAME = ""
    storage_manager = LocalStorageManager()
    content = storage_manager.get_file_locally(TEST_FILE_NAME)
    storage_manager.delete_file_locally(TEST_FILE_NAME)