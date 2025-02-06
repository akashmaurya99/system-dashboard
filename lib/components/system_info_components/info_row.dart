import 'package:flutter/widgets.dart';
import 'package:flutter_svg/flutter_svg.dart';

class InfoRow extends StatelessWidget {
  final String iconPath;
  final String title;
  final List<String>? details;

  const InfoRow(
      {super.key, required this.iconPath, required this.title, this.details});

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        SvgPicture.asset(iconPath, height: 20),
        const SizedBox(width: 10),
        Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(title, style: const TextStyle(fontWeight: FontWeight.bold)),
            if (details != null)
              ...details!.map((detail) =>
                  Text(detail, style: const TextStyle(fontSize: 10))),
          ],
        ),
      ],
    );
  }
}
