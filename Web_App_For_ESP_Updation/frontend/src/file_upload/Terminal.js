import '../App.css';

export default function Terminal(props){
    return(
        <div id="termynal" data-termynal>
    {props.data ?
    <span data-ty="input" data-ty-prompt=">>>">{props.data}</span>:<></> }


</div>
    )
}