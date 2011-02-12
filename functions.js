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
                code = document.getElementById ("bfcode");
                code.value = xmlhttp.responseText;
            }
    }
}