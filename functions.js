function loadBf(filename)
{
    xmlhttp = new XMLHttpRequest ();

    xmlhttp.open("GET", "/brainfuck/" + filename, true);
    xmlhttp.onreadystatechange = stateChangeHandler;
    xmlhttp.send(null);

    function stateChangeHandler()
    {
        if (xmlhttp.readyState == 4)
            {
                document.bfForm.code.value = xmlhttp.responseText;
            }
    }
}