import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:system_info/theme/container_color.dart';
import 'package:system_info/theme/container_radius.dart';
import 'package:system_info/utils/dynamic_size.dart';
import '../components/ram_chart_components/ram_legend.dart';
import '../components/ram_chart_components/ram_provider.dart';

class RamPieChart extends StatelessWidget {
  const RamPieChart({super.key});

  @override
  Widget build(BuildContext context) {
    return Consumer<RamProvider>(
      builder: (context, provider, child) {
        return LayoutBuilder(
          builder: (context, constraints) {
            double widthFactor = 0.24;
            return Container(
              padding:
                  EdgeInsets.symmetric(vertical: dynamicWidth(context) * 0.01),
              height: dynamicHeight(context) * 0.22,
              width: dynamicWidth(context) * widthFactor,
              decoration: BoxDecoration(
                color: ContainerColor.primary,
                borderRadius: ContainerRadius.primary,
              ),
              child: Row(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                children: [
                  AspectRatio(
                    aspectRatio: 1,
                    child: PieChart(
                      PieChartData(
                        pieTouchData: PieTouchData(
                          touchCallback: provider.handleTouch,
                        ),
                        borderData: FlBorderData(show: false),
                        sectionsSpace: 1,
                        centerSpaceRadius: 25,
                        sections: provider.showingSections(context),
                      ),
                    ),
                  ),
                  RamLegend(),
                ],
              ),
            );
          },
        );
      },
    );
  }
}

// import 'package:fl_chart/fl_chart.dart';
// import 'package:flutter/material.dart';
// import 'package:system_info/theme/app_color.dart';
// import 'package:system_info/components/indicator.dart';
// import 'package:system_info/theme/container_color.dart';
// import 'package:system_info/theme/container_radius.dart';
// import 'package:system_info/utils/dynamic_size.dart';

// class RamPiChart extends StatefulWidget {
//   const RamPiChart({
//     super.key,
//   });

//   @override
//   State<RamPiChart> createState() => _RamPiChartState();
// }

// class _RamPiChartState extends State<RamPiChart> {
//   int touchedIndex = -1;
//   @override
//   Widget build(BuildContext context) {
//     return LayoutBuilder(builder: (context, constraints) {
//       return Row(
//         children: [
//           Container(
//             // padding: const EdgeInsets.all(1),
//             height: dynamicHeight(context) * 0.2,
//             decoration: BoxDecoration(
//               color: ContainerColor.primary,
//               borderRadius: ContainerRadius.primary,
//             ),
//             child: Row(
//               children: [
//                 AspectRatio(
//                   aspectRatio: 1,
//                   child: PieChart(
//                     PieChartData(
//                       pieTouchData: PieTouchData(
//                         touchCallback: (FlTouchEvent event, pieTouchResponse) {
//                           setState(() {
//                             if (!event.isInterestedForInteractions ||
//                                 pieTouchResponse == null ||
//                                 pieTouchResponse.touchedSection == null) {
//                               touchedIndex = -1;
//                               return;
//                             }
//                             touchedIndex = pieTouchResponse
//                                 .touchedSection!.touchedSectionIndex;
//                           });
//                         },
//                       ),
//                       borderData: FlBorderData(
//                         show: false,
//                       ),
//                       sectionsSpace: 0,
//                       centerSpaceRadius: 27,
//                       sections: showingSections(),
//                     ),
//                   ),
//                 ),
//                 const Column(
//                   mainAxisAlignment: MainAxisAlignment.end,
//                   crossAxisAlignment: CrossAxisAlignment.start,
//                   children: <Widget>[
//                     SizedBox(
//                       height: 20,
//                     ),
//                     Text("RAM",
//                         style: TextStyle(
//                           fontSize: 16,
//                           fontWeight: FontWeight.w500,
//                           color: Color.fromARGB(175, 255, 255, 255),
//                         )),
//                     Spacer(),
//                     Indicator(
//                       color: AppColors.contentColorBlue,
//                       text: 'System Data',
//                     ),
//                     SizedBox(
//                       height: 4,
//                     ),
//                     Indicator(
//                       color: AppColors.contentColorYellow,
//                       text: 'User Data',
//                     ),
//                     SizedBox(
//                       height: 4,
//                     ),
//                     Indicator(
//                       color: AppColors.contentColorPurple,
//                       text: 'App Data',
//                     ),
//                     SizedBox(
//                       height: 4,
//                     ),
//                     Indicator(
//                       color: AppColors.contentColorGreen,
//                       text: 'Others',
//                     ),
//                     SizedBox(
//                       height: 18,
//                     ),
//                   ],
//                 ),
//                 const SizedBox(
//                   width: 28,
//                 ),
//               ],
//             ),
//           ),
//         ],
//       );
//     });
//   }

//   List<PieChartSectionData> showingSections() {
//     return List.generate(4, (i) {
//       final isTouched = i == touchedIndex;
//       final fontSize = isTouched ? 13.0 : 10.0;
//       final radius = isTouched ? 33.0 : 27.0;
//       const shadows = [Shadow(color: Colors.black, blurRadius: 2)];
//       switch (i) {
//         case 0:
//           return PieChartSectionData(
//             color: AppColors.contentColorBlue,
//             value: 40,
//             title: '40%',
//             radius: radius,
//             titleStyle: TextStyle(
//               fontSize: fontSize,
//               fontWeight: FontWeight.bold,
//               color: AppColors.mainTextColor1,
//               shadows: shadows,
//             ),
//           );
//         case 1:
//           return PieChartSectionData(
//             color: AppColors.contentColorYellow,
//             value: 30,
//             title: '30%',
//             radius: radius,
//             titleStyle: TextStyle(
//               fontSize: fontSize,
//               fontWeight: FontWeight.bold,
//               color: AppColors.mainTextColor1,
//               shadows: shadows,
//             ),
//           );
//         case 2:
//           return PieChartSectionData(
//             color: AppColors.contentColorPurple,
//             value: 15,
//             title: '15%',
//             radius: radius,
//             titleStyle: TextStyle(
//               fontSize: fontSize,
//               fontWeight: FontWeight.bold,
//               color: AppColors.mainTextColor1,
//               shadows: shadows,
//             ),
//           );
//         case 3:
//           return PieChartSectionData(
//             color: AppColors.contentColorGreen,
//             value: 15,
//             title: '15%',
//             radius: radius,
//             titleStyle: TextStyle(
//               fontSize: fontSize,
//               fontWeight: FontWeight.bold,
//               color: AppColors.mainTextColor1,
//               shadows: shadows,
//             ),
//           );
//         default:
//           throw Error();
//       }
//     });
//   }
// }
