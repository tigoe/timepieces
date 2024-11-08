function updateSize() {
  let clockElement = document.getElementById("clock");
  console.log(clockElement);
  clockElement.height = window.innerHeight;
  clockElement.width = window.innerWidth;
  console.log(thisHeight + "," + thisWidth);
}
updateSize();
window.addEventListener('DOMContentLoaded', updateSize);
window.addEventListener("resize", updateSize);