import "../App.css";
import { FilePicker } from "react-file-picker";
import { useState } from "react";
import Button from "@mui/material/Button";
import Alert from "@mui/material/Alert";
import axios from "axios";
import SyntaxHighlighter from "react-syntax-highlighter";
import { docco } from "react-syntax-highlighter/dist/esm/styles/hljs";
import { dark } from "react-syntax-highlighter/dist/esm/styles/prism";
import Terminal from "./Terminal";
import CircularProgress from "@mui/material/CircularProgress";

export default function UploadCode() {
  const [errorMsg, seterrorMsg] = useState("");
  const [errorAlert, seterrorAlert] = useState(false);
  const [uploadMsg, setuploadMsg] = useState("");
  const [uploadAlert, setuploadAlert] = useState(false);
  const [showPreview, setshowPreview] = useState(false);
  const [showConsole, setshowConsole] = useState(false);
  const [file, setFile] = useState("");
  const [terminalLineData, setTerminalLineData] = useState("");
  const [isCompiled, setisCompiled] = useState(false);
  const [showProgress, setshowProgress] = useState(false);
  const [isuploadDisabled, setisuploadDisabled] = useState(true);
  const [iscompileDisabled, setiscompileDisabled] = useState(true);
  const showFile = (FileObject) => {
    setFile(FileObject);
    setshowPreview(true);
    setisuploadDisabled(false);
    var reader = new FileReader();
    reader.onload = function (event) {
      document.getElementById("show-text").innerText = event.target.result;
    };
    reader.readAsText(FileObject);
  };

  const compileFile = () => {
    setshowProgress(true);
    setshowPreview(false);
    if (file === "") {
      seterrorAlert(true);
      seterrorMsg("No file selected");
      return;
    }
    axios.get("/api/compile").then(function (response) {
      console.log(response);
      setisuploadDisabled(true);
      setiscompileDisabled(true);
      setshowProgress(false);
      setshowConsole(true);
      seterrorAlert(false);
      setisCompiled(true);
      setTerminalLineData(response.data.output["cli-msg"]);
    });
  };

  const uploadFile = () => {
    if (file === "") {
      seterrorAlert(true);
      seterrorMsg("No file selected");
      return;
    }

    const data = new FormData();
    data.append("file", file);
    axios.post("/api/upload", data).then(function (response) {
      console.log(response);
      if (response["status"] === 200) {
        seterrorAlert(false);
        setuploadAlert(true);
        setuploadMsg("File Uploaded Successfully");
        setiscompileDisabled(false);
        setTimeout(function () {
          setuploadAlert(false);
        }, 5000);
      } else {
        seterrorAlert(true);
        seterrorMsg(response);
      }
    });
  };
  return (
    <>
      <div className="App">
        <header className="App-header">
          <p>Upload your Arduino file here.</p>
          <br />
          {errorAlert ? <Alert severity="error">{errorMsg}</Alert> : <></>}

          <br />

          <FilePicker
            extensions={["cpp", "ino"]}
            onChange={(FileObject) => (
              showFile(FileObject), seterrorAlert(false), setshowConsole(false)
            )}
            onError={(errMsg) => (
              seterrorAlert(true), seterrorMsg(errMsg), setshowPreview(false)
            )}
          >
            <div className="container">
              <Button variant="contained" component="span">
                Choose File
              </Button>
            </div>
          </FilePicker>
          <br />
          {uploadAlert ? <Alert severity="success">{uploadMsg}</Alert> : <></>}
          <br />
          <div className="container">
            <Button
              variant="contained"
              component="span"
              onClick={uploadFile}
              disabled={isuploadDisabled}
            >
              Upload File
            </Button>
            &nbsp;
            <Button
              variant="contained"
              component="span"
              onClick={compileFile}
              disabled={iscompileDisabled}
            >
              Compile & Send
            </Button>
          </div>
          {showPreview ? (
            <>
              <div className="preview">
                <SyntaxHighlighter
                  language="cpp"
                  style={dark}
                  id="show-text"
                ></SyntaxHighlighter>
              </div>
            </>
          ) : (
            <></>
          )}
          {showProgress ? (
            <>
              <br /> <CircularProgress />{" "}
            </>
          ) : showConsole ? (
            <Terminal data={terminalLineData} />
          ) : (
            <></>
          )}

          <br />
        </header>
      </div>
    </>
  );
}
