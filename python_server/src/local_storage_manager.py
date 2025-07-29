import os

class LocalStorageManager:
    def __init__(self):
        pass

    def save_file_locally(self, file_name, file_content):
        with open(file_name, 'wb') as file:
            file.write(file_content)

    def get_file_locally(self, file_name):
        try:
            with open(file_name, 'rb') as file:
                return file.read()
        except FileNotFoundError:
            print(f"File {file_name} does not exist. Cannot read.")
            return None
        except Exception as e:
            print(f"Unexpected error while reading {file_name} , error: {e}")
        return None
    def delete_file_locally(self, file_name):
        try:
            os.remove(file_name)
            print(f"File {file_name} deleted successfully.")
        except FileNotFoundError:
            print(f"File {file_name} does not exist. Cannot delete.")
        except Exception as e:
            print(f"Unexpected error while deleting {file_name}, error: {e}")
        return None

if __name__ == "__main__":

    storage_manager = LocalStorageManager()

    content = storage_manager.get_file_locally("dog_run_173.csv")
    storage_manager.delete_file_locally("dog_run_173.csv")