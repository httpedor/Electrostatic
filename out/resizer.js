window.addEventListener("resize", () =>{
    var w = window.innerWidth;
    var h = window.innerHeight;
    var canvas = document.getElementById("canvas");
    canvas.width = w;
    canvas.height = h;
});