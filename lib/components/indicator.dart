import 'package:flutter/widgets.dart';

class Indicator extends StatelessWidget {
  const Indicator({
    super.key,
    required this.color,
    required this.text,
    this.size = 12,
    this.textColor,
  });
  final Color color;
  final String text;
  final double size;
  final Color? textColor;

  @override
  Widget build(BuildContext context) {
    return Row(
      children: <Widget>[
        Container(
          width: size,
          height: size,
          decoration: BoxDecoration(
            borderRadius: BorderRadius.circular(3),
            color: color,
          ),
        ),
        const SizedBox(
          width: 4,
        ),
        Text(
          text,
          style: TextStyle(
            fontSize: 12,
            fontWeight: FontWeight.w400,
            color: textColor,
          ),
        )
      ],
    );
  }
}
