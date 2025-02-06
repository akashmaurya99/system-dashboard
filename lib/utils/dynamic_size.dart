import 'package:flutter/widgets.dart';

double dynamicWidth(context) {
  double width = MediaQuery.of(context).size.width;
  return width;
}

double dynamicHeight(context) {
  double height = MediaQuery.of(context).size.height;
  return height;
}
