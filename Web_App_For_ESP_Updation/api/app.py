import os
import requests
from hashlib import md5
from asyncio.subprocess import PIPE
import subprocess
from flask import send_file, Flask, request, jsonify, make_response
from flask_cors import CORS
from werkzeug.utils import secure_filename

app = Flask(__name__)
UPLOAD_FOLDER = r"assets"
ALLOWED_EXTENSIONS = set(['ino', 'cpp'])
boards = {
	'UNO': 'arduino:avr:uno',
	'ESP': 'esp8266:esp8266:nodemcu'
}
build_path = {
	'UNO': 'arduino.avr.uno',
	'ESP': 'esp8266.esp8266.nodemcu'
}
cors = CORS(app, resources={
	r"/*": {
			"origins": "*",
			"Access-Control-Allow-Origin": "*"
			}
})

@app.route('/test/checkUpdate', methods=['GET'])
def checkUpdate():
	return make_response(jsonify({'available': 1}))

@app.route('/test/sendFile', methods=['GET'])
def sendFile():
	bin_path = ''
	try:
		return send_file(bin_path)
	except Exception as e:
		return str(e)

@app.route('/test/checkMD5', methods=['GET'])
def checkMD5():
	bin_path = ''
	md5_hash = md5()
	bin_file = open(bin_path, "rb")
	bin_content = bin_file.read()
	bin_file.close()
	md5_hash.update(bin_content)
	MD5Hash = md5_hash.hexdigest()
	data = {'MD5': MD5Hash}
	return make_response(jsonify(data), 200)

# @app.route('/test/updateFirmware', methods=['GET'])
def updateFirmware(bin_path):
	cookies = {
		'visited': '1',
	}
	headers = {
		'Accept': '*/*',
		'Accept-Language': 'en-US,en;q=0.9',
		'Connection': 'keep-alive',
		'DNT': '1',
		'Origin': 'http://192.168.29.53',
		'Referer': 'http://192.168.29.53/update',
		'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/101.0.0.0 Safari/537.36',
	}

	md5_hash = md5()
	bin_file = open(bin_path, "rb")
	bin_content = bin_file.read()
	bin_file.close()
	md5_hash.update(bin_content)
	MD5Hash = md5_hash.hexdigest()
	data = {'MD5': MD5Hash}
	bin_file = open(bin_path, "rb")

	# data = '------WebKitFormBoundaryUi4EPMcP5Kzd0MgY\r\nContent-Disposition: form-data; name="MD5"\r\n\r\na6983304d8bfd9b033ce7234f614315b\r\n------WebKitFormBoundaryUi4EPMcP5Kzd0MgY\r\nContent-Disposition: form-data; name="firmware"; filename="firmware"\r\nContent-Type: application/octet-stream\r\n\r\n\r\n------WebKitFormBoundaryUi4EPMcP5Kzd0MgY--\r\n'
	files = [('firmware', ('firmware', bin_file,
						   'application/octet-stream; boundary=----WebKitFormBoundaryUi4EPMcP5Kzd0MgY'))]
	try:
		response = requests.post('http://192.168.29.53/update', cookies=cookies,
							 headers=headers, data=data, files=files, timeout=200,)
	except requests.exceptions.ConnectionError:
		bin_file.close()
		return	"Device is restarting!!!"
	bin_file.close()
	return response

@app.route('/api/compile', methods=['GET'])
def compile():
	board = 'ESP'
	bin_path = 'assets/code/build/{}/code.ino.bin'.format(
		build_path[board])
	CMD_command = "arduino-cli compile --fqbn {} assets\code -e".format(
		boards[board])

	# result = subprocess.run(['dir'],stdout=subprocess.PIPE)
	# result = subprocess.run(['arduino-cli', 'compile', '--fqbn', 'arduino:avr:uno' ,'assets/code/Blink.ino'  ,'-e'], stdout=subprocess.PIPE)
	result = subprocess.run(
		CMD_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

	# print(result.stdout)
	# print(result.stderr.decode('utf-8'))

	if result.returncode:
		return make_response(jsonify({'output': {'status': 'Failed', 'cli-msg': result.stderr.decode('utf-8')}}), 200)

	response = updateFirmware(bin_path)

	return make_response(jsonify({'output': {'status': 'Success', 'cli-msg': result.stdout.decode('utf-8'), 'update-status': response}}), 200)

@app.route('/api/upload', methods=['POST'])
def upload():
	if('file' in request.files):
		target = os.path.join(UPLOAD_FOLDER, 'code')
		file = request.files['file']
		filename = secure_filename(file.filename)
		name, extension = os.path.splitext(filename)
		destination = "\\".join([target, 'code'+extension])
		file.save(destination)
		print("File saved to: " + destination)
		return make_response(
			'success',
			200)
	return make_response(
		'Failed',
		202)

@app.route('/')
def home():
	return 'Welcome to Over-The-Air Firmaware Update Utility Kit'

if __name__ == '__main__':
	app.run(debug=True)
