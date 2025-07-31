import csv
import io
import os


RAW_ESP32_FILES_DIR = "raw_esp32_files"

class GPXConverter:
    def __init__(self, creator="Dog Collar GPS"):
        self.creator = creator


    def parse_csv_file(self, file, file_name):
        
        try:
            text = file.decode('utf-8')
            reader = csv.DictReader(io.StringIO(text))
            points = []

            for row in reader:
                # Only take timestamp, latitude, longitude
                points.append({
                    'time': row['timestamp'],
                    'lat': row['latitude'],
                    'lon': row['longitude']
                })

            return True, points
        
        except Exception as e:
            print(f"Error parsing CSV file {file_name}: {e}")
            return False, []

    def add_gpx_xml_declaration(self):
        return '<?xml version="1.0" encoding="UTF-8"?>\n'
    
    def add_gpx_root_element(self):
        return f'<gpx version="1.1" creator="{self.creator}" xmlns="http://www.topografix.com/GPX/1/1">\n'

    def add_gpx_start_trk_element(self, file_name):
        return f'  <trk> \n' \
               f'    <name>{file_name}</name>\n' \
               f'    <trkseg>\n'
    
    def add_gpx_trkpt_element(self, lat, lon, time):
        return f'      <trkpt lat="{lat}" lon="{lon}">\n' \
               f'        <time>{time}</time>\n' \
               f'      </trkpt>\n'
    
    def add_gpx_end_trk_element(self):
        return f'    </trkseg>\n' \
               f'  </trk>\n'
    
    def add_gpx_end_element(self):
        return '</gpx>\n'
    
    def convert_to_gpx(self, file, file_name):

        success, points = self.parse_csv_file(file, file_name)

        if not success:
            print(f"Failed to parse file {file_name}.")
            return None
        if not points:
            print(f"No points found in file {file_name}.")
            return None



        gpx_content = self.add_gpx_xml_declaration()
        gpx_content += self.add_gpx_root_element()
        gpx_content += self.add_gpx_start_trk_element(file_name)
        
        for point in points:
            lat, lon, time = point['lat'], point['lon'], point['time']
            gpx_content += self.add_gpx_trkpt_element(lat, lon, time)
        
        gpx_content += self.add_gpx_end_trk_element()
        gpx_content += self.add_gpx_end_element()


        
        return gpx_content
