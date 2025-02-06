import 'package:flutter/material.dart';

class ProgramTitle extends StatelessWidget {
  const ProgramTitle({super.key});

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceAround,
      children: [
        Text("Name"),
        Text("CPU Uses"),
        Text("RAM Uses"),
        Text("Storage uses")
      ],
    );
  }
}
